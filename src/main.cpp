#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_opengl_glext.h>
#include <SDL2/SDL_ttf.h>

#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <cstring>
#include <cstdio>
#include <fstream>

#include "hmp/hmp.h"
#include "hmp/hmp_box_t.h"

#include "timer_t.h"
#include "memory_t.h"
#include "path_t.h"
#include "platform.h"
#include "util.h"
#include "consts.h"

#if HMP_CAPTURE_ENABLED == ON
#define LLCE_CAPTURE 1
#endif

#ifdef LLCE_CAPTURE
extern "C" {
#include <png.h>

#include <libavcodec/avcodec.h>
#include <libavutil/common.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
}
#endif

typedef void (*init_f)( hmp::state_t*, hmp::input_t* );
typedef void (*boot_f)( hmp::graphics_t* );
typedef void (*update_f)( hmp::state_t*, hmp::input_t*, const float64_t );
typedef void (*render_f)( const hmp::state_t*, const hmp::input_t*, const hmp::graphics_t* );
typedef std::ios_base::openmode ioflag_t;
typedef llce::platform::path_t path_t;
typedef llce::util::color_t color_t;

int32_t main( const int32_t pArgCount, const char8_t* pArgs[] ) {
    /// Initialize Global Constant State ///

    const float64_t cSimFPS = 60.0;

    /// Parse Input Arguments ///

    if( llce::cli::exists("-v", pArgs, pArgCount) ) {
        LLCE_ALERT_INFO( "{Version: v0.0.a, " <<
            "Build: " << (LLCE_DEBUG ? "Debug" : "Release") << ", " <<
            "Capture*:" << (LLCE_CAPTURE ? "Enabled" : "Disabled") << "}" );
    }

    const char8_t* cSimStateArg = llce::cli::value( "-r", pArgs, pArgCount );
    const int32_t cSimStateIdx = cSimStateArg != nullptr ? std::atoi( cSimStateArg ) : -1;
#ifdef LLCE_DEBUG
    bool32_t cIsSimulating = cSimStateIdx > 0;
#else
    bool32_t cIsSimulating = false;
#endif

    /// Initialize Application Memory/State ///

    // NOTE(JRC): This base address was chosen by following the steps enumerated
    // in the 'doc/static_address.md' documentation file.
#ifdef LLCE_DEBUG
    bit8_t* const cBufferAddress = (bit8_t*)0x0000100000000000;
    const uint64_t cBackupBufferCount = static_cast<uint64_t>( 2.0 * cSimFPS );
#else
    bit8_t* const cBufferAddress = nullptr;
    const uint64_t cBackupBufferCount = 0;
#endif
    const uint64_t cSimBufferIdx = 0, cSimBufferLength = MEGABYTE_BL( 1 );
    const uint64_t cBackupBufferIdx = 1, cBackupBufferLength = cBackupBufferCount * cSimBufferLength;
    const uint64_t cBufferLengths[2] = { cSimBufferLength, cBackupBufferLength };

    llce::memory_t mem( 2, &cBufferLengths[0], cBufferAddress );
    hmp::input_t* inputs = (hmp::input_t*)mem.allocate( cSimBufferIdx, 2 * sizeof(hmp::input_t) );

    hmp::state_t* simState = (hmp::state_t*)mem.allocate( cSimBufferIdx, sizeof(hmp::state_t) );
    hmp::input_t* simInput = &inputs[0];
    hmp::graphics_t* simGraphics = (hmp::graphics_t*)mem.allocate( cSimBufferIdx, sizeof(hmp::graphics_t) );

#ifdef LLCE_DEBUG
    hmp::input_t* backupInputs = (hmp::input_t*)mem.allocate( cBackupBufferIdx, cBackupBufferCount * sizeof(hmp::input_t) );
    hmp::state_t* backupStates = (hmp::state_t*)mem.allocate( cBackupBufferIdx, cBackupBufferCount * sizeof(hmp::state_t) );
#endif

    std::fstream recStateStream, recInputStream;
    const ioflag_t cIOModeR = std::fstream::binary | std::fstream::in;
    const ioflag_t cIOModeW = std::fstream::binary | std::fstream::out | std::fstream::trunc;

    /// Find Project Paths ///

    const path_t cExePath = llce::platform::exeBasePath();
    LLCE_ASSERT_ERROR( cExePath.exists(),
        "Failed to find path to running executable." );

    const path_t cInstallPath( 2, cExePath.cstr(), path_t::DUP );
    LLCE_ASSERT_ERROR( cInstallPath.exists(),
        "Failed to find path to running executable." );
    const path_t cInstallLockPath( 2, cInstallPath.cstr(), "install.lock" );

    // NOTE(JRC): It's important to realize that the pathing here ties debugging
    // outputs to particular builds of the code. This is probably a good thing, but
    // it does mean clearing the current build will cause all debug files to be lost.
    const path_t cOutputPath( 2, cInstallPath.cstr(), "out" );
    const char8_t* cStateFileFormat = "state%u.dat";
    const char8_t* cInputFileFormat = "input%u.dat";
    const char8_t* cRenderFileFormat = "render%u.png";
    const char8_t* cReplayFileFormat = "replay.mp4";
    const static int32_t csOutputFileNameLength = 20;

    /// Load Dynamic Shared Library ///

    const char8_t* cDLLFileName = "libhmp.so";
    const path_t cDLLPath = llce::platform::libFindDLLPath( cDLLFileName );
    LLCE_ASSERT_ERROR( cDLLPath.exists(),
        "Failed to find library " << cDLLFileName << " in dynamic path." );

    void* dllHandle = nullptr;
    init_f dllInit = nullptr;
    boot_f dllBoot = nullptr;
    update_f dllUpdate = nullptr;
    render_f dllRender = nullptr;
    const auto cDLLReload = [ &cDLLPath, &dllHandle, &dllInit, &dllBoot, &dllUpdate, &dllRender ] () {
        if( dllHandle != nullptr ) {
            llce::platform::dllUnloadHandle( dllHandle, cDLLPath );
        }

        dllHandle = llce::platform::dllLoadHandle( cDLLPath );
        dllInit = (init_f)llce::platform::dllLoadSymbol( dllHandle, "init" );
        dllBoot = (boot_f)llce::platform::dllLoadSymbol( dllHandle, "boot" );
        dllUpdate = (update_f)llce::platform::dllLoadSymbol( dllHandle, "update" );
        dllRender = (render_f)llce::platform::dllLoadSymbol( dllHandle, "render" );

        return dllHandle != nullptr &&
            dllInit != nullptr && dllBoot != nullptr &&
            dllUpdate != nullptr && dllRender != nullptr;
    };

    LLCE_ASSERT_ERROR( cDLLReload(),
        "Couldn't load library `" << cDLLFileName << "` symbols on initialize." );

    int64_t prevDylibModTime, currDylibModTime;
    LLCE_ASSERT_ERROR(
        prevDylibModTime = currDylibModTime = cDLLPath.modtime(),
        "Couldn't load library `" << cDLLFileName << "` stat data on initialize." );

    /// Initialize Windows/Graphics ///

    // TODO(JRC): Include 'SDL_INIT_GAMECONTROLLER' when it's needed; it causes
    // extra one-time leaks so it has been excluded to aid in memory error tracking.
    LLCE_ASSERT_ERROR(
        SDL_Init(SDL_INIT_VIDEO) >= 0,
        "SDL failed to initialize; " << SDL_GetError() );

    LLCE_ASSERT_ERROR(
        TTF_Init() >= 0,
        "SDL-TTF failed to initialize; " << TTF_GetError() );

    { // Initialize OpenGL Context //
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2 );

        SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 ); // double-buffer
        SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 32 );

        SDL_GL_SetSwapInterval( 1 ); // vsync
    }

    int32_t windowWidth = 640, windowHeight = 480;
    const uint32_t cWindowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE |
        ( cIsSimulating ? SDL_WINDOW_HIDDEN : 0 );
    SDL_Window* window = SDL_CreateWindow(
        "Handmade Pong",                            // Window Title
        SDL_WINDOWPOS_UNDEFINED,                    // Window X Position
        SDL_WINDOWPOS_UNDEFINED,                    // Window Y Position
        windowWidth,                                // Window Width
        windowHeight,                               // Window Height
        cWindowFlags );                             // Window Flags
    LLCE_ASSERT_ERROR( window != nullptr,
        "SDL failed to create window instance; " << SDL_GetError() );

    SDL_GLContext glcontext = SDL_GL_CreateContext( window );
    LLCE_ASSERT_ERROR( glcontext != nullptr,
        "SDL failed to generate OpenGL context; " << SDL_GetError() );

    { // Load OpenGL Extensions //
        const static char8_t* csGLExtensionNames[] = { "GL_EXT_framebuffer_object", "GL_EXT_framebuffer_blit" };
        const static uint32_t csGLExtensionCount = ARRAY_LEN( csGLExtensionNames );
        for( uint32_t extensionIdx = 0; extensionIdx < csGLExtensionCount; ++extensionIdx ) {
            const char8_t* glExtensionName = csGLExtensionNames[extensionIdx];
            LLCE_ASSERT_ERROR( SDL_GL_ExtensionSupported(glExtensionName),
                "Failed to load OpenGL extension '" << glExtensionName << "'." );
        }
    }

    { // Configure OpenGL Context //
        glEnable( GL_DEPTH_TEST );
        glDepthFunc( GL_ALWAYS );

        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

        glEnable( GL_TEXTURE_2D );
        glDisable( GL_LIGHTING );

        glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
        glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
    }

    /// Generate Graphics Assets ///

    const char8_t* cFontFileName = "dejavu_mono.ttf";
    const path_t cFontPath( 3, cInstallPath.cstr(), "dat", cFontFileName );
    LLCE_ASSERT_ERROR( cFontPath.exists(),
        "Failed to locate font with file name '" << cFontFileName << "'." );

    const int32_t cFontSize = 20;
    TTF_Font* font = TTF_OpenFont( cFontPath, cFontSize );
    LLCE_ASSERT_ERROR( font != nullptr,
        "SDL-TTF failed to create font; " << TTF_GetError() );

#ifdef LLCE_DEBUG
    const static uint32_t csTextureTextLength = 20;
    uint32_t textureGLIDs[] = { 0, 0, 0, 0 };
    color_t textureColors[] = { {0xFF, 0x00, 0x00, 0xFF}, {0x00, 0xFF, 0x00, 0xFF}, {0x00, 0x00, 0xFF, 0xFF} };
    char8_t textureTexts[][csTextureTextLength] = { "FPS: ???", "Recording ???", "Replaying ???", "Time: ???" };
    const uint32_t cFPSTextureID = 0, cRecTextureID = 1, cRepTextureID = 2, cTimeTextureID = 3;

    const uint32_t cTextureCount = ARRAY_LEN( textureGLIDs );
    for( uint32_t textureIdx = 0; textureIdx < cTextureCount; textureIdx++ ) {
        uint32_t& textureGLID = textureGLIDs[textureIdx];
        glGenTextures( 1, &textureGLID );
        glBindTexture( GL_TEXTURE_2D, textureGLID );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
    }

    // NOTE(JRC): This method for generating textures is expedient in terms of
    // development time, but suboptimal in terms of performance. Every time a texture
    // is generated using this function, the SDL library allocates memory into
    // dynamically allocated buffers, which are immediately freed after this data
    // is stored in memory. Since the texture buffers are all statically sized, it
    // would be ideal if the same, statically allocated arrays were filled in this
    // method, but customizing memory allocations for SDL isn't easy to do. For a
    // performance-level texture generation method, watch the "Handmade Hero" tutorials
    // on OpenGL texturing and font APIs.
    const auto cGenerateTextTexture = [ &textureGLIDs, &font ]
            ( const uint32_t pTextureID, const color_t pTextureColor, const char8_t* pTextureText ) {
        const uint32_t& textureGLID = textureGLIDs[pTextureID];

        SDL_Color renderColor = { pTextureColor.r, pTextureColor.g, pTextureColor.b, pTextureColor.a };
        SDL_Surface* textSurface = TTF_RenderText_Solid( font, pTextureText, renderColor );
        LLCE_ASSERT_ERROR( textSurface != nullptr,
            "SDL-TTF failed to render font; " << TTF_GetError() );
        SDL_Surface* renderSurface = SDL_ConvertSurfaceFormat( textSurface, SDL_PIXELFORMAT_RGBA8888, 0 );
        LLCE_ASSERT_ERROR( renderSurface != nullptr,
            "SDL failed to convert render font output; " << SDL_GetError() );

        glBindTexture( GL_TEXTURE_2D, textureGLID );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, renderSurface->w, renderSurface->h,
            0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, renderSurface->pixels );

        SDL_FreeSurface( renderSurface );
        SDL_FreeSurface( textSurface );
    };

    for( uint32_t textureIdx = 0; textureIdx < cTextureCount; textureIdx++ ) {
        cGenerateTextTexture( textureIdx, textureColors[textureIdx], textureTexts[textureIdx] );
    }
#endif

    /// Input Wrangling ///

    // NOTE(JRC): This is the true input stream for the application.
    // The simulation stream is kept separate so its inputs don't interfere
    // with application-level functionality (e.g. debugging contexts, etc.).
    hmp::input_t* appInput = &inputs[1];

    const SDL_Scancode cFXKeyGroup[] = {
        SDL_SCANCODE_F1, SDL_SCANCODE_F2, SDL_SCANCODE_F3, SDL_SCANCODE_F4,
        SDL_SCANCODE_F5, SDL_SCANCODE_F6, SDL_SCANCODE_F7, SDL_SCANCODE_F8,
        SDL_SCANCODE_F9, SDL_SCANCODE_F10, SDL_SCANCODE_F11, SDL_SCANCODE_F12
    };
    const uint32_t cFXKeyGroupSize = ARRAY_LEN( cFXKeyGroup );

    const auto cIsKeyDown = [ &appInput ] ( const SDL_Scancode pKey ) {
        return (bool32_t)( appInput->keys[pKey] );
    };
    const auto cIsKGDown = [ &cIsKeyDown ]
            ( const SDL_Scancode* pKeyGroup, const uint32_t pGroupSize ) {
        uint32_t firstIdx = 0;
        for( uint32_t groupIdx = 0; groupIdx < pGroupSize && firstIdx == 0; groupIdx++ ) {
            firstIdx = cIsKeyDown( pKeyGroup[groupIdx] ) ? groupIdx + 1 : 0;
        }
        return firstIdx;
    };

    const auto cWasKeyPressed = [ &appInput ] ( const SDL_Scancode pKey ) {
        return (bool32_t)( appInput->keys[pKey] && appInput->diffs[pKey] == hmp::KEY_DIFF_DOWN );
    };
    const auto cWasKGPressed = [ &cWasKeyPressed ]
            ( const SDL_Scancode* pKeyGroup, const uint32_t pGroupSize ) {
        uint32_t firstIdx = 0;
        for( uint32_t groupIdx = 0; groupIdx < pGroupSize && firstIdx == 0; groupIdx++ ) {
            firstIdx = cWasKeyPressed( pKeyGroup[groupIdx] ) ? groupIdx + 1 : 0;
        }
        return firstIdx;
    };

    const auto cWasKeyReleased = [ &appInput ] ( const SDL_Scancode pKey ) {
        return (bool32_t)( appInput->keys[pKey] && appInput->diffs[pKey] == hmp::KEY_DIFF_UP );
    };
    const auto cWasKGReleased = [ &cWasKeyReleased ]
            ( const SDL_Scancode* pKeyGroup, const uint32_t pGroupSize ) {
        uint32_t firstIdx = 0;
        for( uint32_t groupIdx = 0; groupIdx < pGroupSize && firstIdx == 0; groupIdx++ ) {
            firstIdx = cWasKeyReleased( pKeyGroup[groupIdx] ) ? groupIdx + 1 : 0;
        }
        return firstIdx;
    };

    /// Update/Render Loop ///

    bool32_t isRunning = true, isStepping = false;

    bool32_t isRecording = false, isReplaying = false;
    uint32_t currSlotIdx = 0, recSlotIdx = 0;
    uint32_t repFrameIdx = 0, recFrameCount = 0;

    bool32_t isCapturing = LLCE_CAPTURE && cIsSimulating;
    uint32_t currCaptureIdx = 0;

    llce::timer_t simTimer( cSimFPS, llce::timer_t::ratio_e::fps );
    float64_t simDT = 0.0;
    // NOTE(JRC): A cursory check shows that it will take ~1e10 years of
    // uninterrupted run time for this to overflow at 60 FPS, so the fact
    // that this increments very quickly over time isn't a big concern.
    uint64_t simFrame = 0;

    dllInit( simState, simInput );
    dllBoot( simGraphics );
    while( isRunning ) {
#ifdef LLCE_DEBUG
        bool32_t isStepReady = false;
        while( isStepping && !isStepReady ) {
            SDL_Event event;
            SDL_WaitEvent( &event );
            if( event.type == SDL_QUIT || (
                    event.type == SDL_KEYDOWN && (
                    event.key.keysym.scancode == SDL_SCANCODE_RETURN ||
                    event.key.keysym.scancode == SDL_SCANCODE_SPACE)) ) {
                isStepReady = true;
                SDL_PushEvent( &event );
            }
        }
#endif
        simTimer.split();

        const uint8_t* keyboardState = SDL_GetKeyboardState( nullptr );
        for( uint32_t keyIdx = 0; keyIdx < sizeof(appInput->keys); keyIdx++ ) {
            const bool8_t wasKeyDown = appInput->keys[keyIdx];
            const bool8_t isKeyDown = keyboardState[keyIdx];

            appInput->keys[keyIdx] = isKeyDown;
            appInput->diffs[keyIdx] = (
                (!wasKeyDown && isKeyDown) ? hmp::KEY_DIFF_DOWN : (
                (wasKeyDown && !isKeyDown) ? hmp::KEY_DIFF_UP : (
                hmp::KEY_DIFF_NONE)) );
        }
        std::memcpy( simInput, appInput, sizeof(hmp::input_t) );

        SDL_Event event;
        while( SDL_PollEvent(&event) ) {
            if( event.type == SDL_QUIT ) {
                isRunning = false;
            } else if( event.type == SDL_WINDOWEVENT  && (
                   event.window.event == SDL_WINDOWEVENT_RESIZED ||
                   event.window.event == SDL_WINDOWEVENT_EXPOSED) ) {
                SDL_GetWindowSize( window, &windowWidth, &windowHeight );
            }
        }

        if( cIsKeyDown(SDL_SCANCODE_Q) ) {
            // q key = quit application
            isRunning = false;
        } if( cWasKeyPressed(SDL_SCANCODE_GRAVE) ) {
            // ` key = capture application
            isCapturing = true;
        }
#ifdef LLCE_DEBUG
        uint64_t backupIdx = simFrame % cBackupBufferCount;
        std::memcpy( (void*)&backupInputs[backupIdx], (void*)appInput, sizeof(hmp::input_t) );
        std::memcpy( (void*)&backupStates[backupIdx], (void*)simState, sizeof(hmp::state_t) );

        if( (!isStepping && cWasKeyPressed(SDL_SCANCODE_SPACE)) ||
                (isStepping && cIsKeyDown(SDL_SCANCODE_SPACE)) ) {
            // space key = toggle frame advance mode
            LLCE_ALERT_INFO( "Frame Advance <" << (!isStepping ? "ON " : "OFF") << ">" );
            isStepping = !isStepping;
        }

        if( (currSlotIdx = cWasKGPressed(&cFXKeyGroup[0], cFXKeyGroupSize)) || (cIsSimulating && !isReplaying) ) {
            // function key (fx) = debug state operation
            recSlotIdx = !cIsSimulating ? currSlotIdx : cSimStateIdx;

            char8_t slotStateFileName[csOutputFileNameLength];
            char8_t slotInputFileName[csOutputFileNameLength];

            std::snprintf( &slotStateFileName[0], sizeof(slotStateFileName),
                cStateFileFormat, recSlotIdx );
            std::snprintf( &slotInputFileName[0], sizeof(slotInputFileName),
                cInputFileFormat, recSlotIdx );

            path_t slotStateFilePath( 2, cOutputPath.cstr(), &slotStateFileName[0] );
            path_t slotInputFilePath( 2, cOutputPath.cstr(), &slotInputFileName[0] );

            if( (cIsKeyDown(SDL_SCANCODE_LSHIFT) && !isRecording) || cIsSimulating ) {
                // lshift + fx = toggle slot x replay
                LLCE_ALERT_INFO( "Replay Slot {" << recSlotIdx << "} <" << (!isReplaying ? "ON " : "OFF") << ">" );
                if( !isReplaying ) {
                    repFrameIdx = 0;
                    recStateStream.open( slotStateFilePath, cIOModeR );
                    recInputStream.open( slotInputFilePath, cIOModeR );

                    recFrameCount = (uint32_t)recInputStream.tellg();
                    recInputStream.seekg( 0, std::ios_base::end );
                    recFrameCount = (uint32_t)recInputStream.tellg() - recFrameCount;
                    recFrameCount /= sizeof( hmp::input_t );
                } else {
                    repFrameIdx = 0;
                    recStateStream.close();
                    recInputStream.close();
                }
                isReplaying = !isReplaying;
            } else if( cIsKeyDown(SDL_SCANCODE_RSHIFT) && !isRecording ) {
                // rshift + fx = hotload slot x state (reset replay)
                LLCE_ALERT_INFO( "Hotload Slot {" << recSlotIdx << "}" );
                if( isReplaying ) {
                    repFrameIdx = 0;
                    recInputStream.seekg( 0, std::ios_base::end );
                } else {
                    recStateStream.open( slotStateFilePath, cIOModeR );
                    recInputStream.seekg( 0 );
                    recStateStream.read( (bit8_t*)simState, sizeof(hmp::state_t) );
                    recStateStream.close();
                }
            } else if( recSlotIdx != 1 && !isReplaying ) {
                // fx = toggle slot x recording
                LLCE_ALERT_INFO( "Record Slot {" << recSlotIdx << "} <" << (!isRecording ? "ON " : "OFF") << ">" );
                if( !isRecording ) {
                    recFrameCount = 0;
                    recStateStream.open( slotStateFilePath, cIOModeW );
                    recStateStream.write( (bit8_t*)simState, sizeof(hmp::state_t) );
                    recStateStream.close();
                    recInputStream.open( slotInputFilePath, cIOModeW );
                } else {
                    recInputStream.close();
                }
                isRecording = !isRecording;
            } else if(recSlotIdx == 1 && !isReplaying ) {
                // f1 = instant backup record
                LLCE_ALERT_INFO( "Hotsave Slot {" << recSlotIdx << "}" );
                // TODO(JRC): It's potentially worth putting a guard on this
                // function or improving the backup state implementation so
                // that hot-saving before the number of total backups is possible.
                uint64_t backupStartIdx = ( backupIdx + 1 ) % cBackupBufferCount;
                recStateStream.open( slotStateFilePath, cIOModeW );
                recStateStream.write( (bit8_t*)&backupStates[backupStartIdx], sizeof(hmp::state_t) );
                recStateStream.close();

                recInputStream.open( slotInputFilePath, cIOModeW );
                for( uint32_t bufferIdx = 0; bufferIdx < cBackupBufferCount; bufferIdx++ ) {
                    uint64_t bbIdx = (backupStartIdx + bufferIdx) % cBackupBufferCount;
                    recInputStream.write( (bit8_t*)&backupInputs[bbIdx], sizeof(hmp::input_t) );
                }
                recInputStream.close();
            }
        }

        if( isRecording ) {
            recInputStream.write( (bit8_t*)simInput, sizeof(hmp::input_t) );
            recFrameCount++;
        } if( isReplaying ) {
            if( recInputStream.peek() == EOF || recInputStream.eof() ) {
                isRunning = !( cIsSimulating && repFrameIdx != 0 );
                repFrameIdx = 0;
                recStateStream.seekg( 0 );
                recStateStream.read( (bit8_t*)simState, sizeof(hmp::state_t) );
                recInputStream.seekg( 0 );
            }
            recInputStream.read( (bit8_t*)simInput, sizeof(hmp::input_t) );
            repFrameIdx++;
        }

        LLCE_ASSERT_ERROR(
            currDylibModTime = cDLLPath.modtime(),
            "Couldn't load library `" << cDLLFileName << "` stat data on step." );
        if( currDylibModTime != prevDylibModTime ) {
            // TODO(JRC): This isn't ideal since spinning in this way can really
            // ramp up processing time, but it's a permissible while there aren't
            // too many iterations per reload (there are none at time of writing).
            uint32_t lockSpinCount = 0;
            while( cInstallLockPath.exists() ) { lockSpinCount++; }

            LLCE_ASSERT_INFO( lockSpinCount == 0,
                "Performed " << lockSpinCount << " spin cycles " <<
                "while waiting for DLL install; consider transitioning to file locks." );

            LLCE_ASSERT_ERROR( cDLLReload(),
                "Couldn't load library `" << cDLLFileName << "` symbols at " <<
                "simulation time " << simTimer.tt() << "." );

            LLCE_ALERT_INFO( "DLL Reload {" << simFrame << "}" );

            prevDylibModTime = currDylibModTime;
        }
#endif
        glViewport( 0, 0, windowWidth, windowHeight );

        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();
        glOrtho( -1.0f, +1.0f, -1.0f, +1.0f, -1.0f, +1.0f );
        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity();

        dllUpdate( simState, simInput, simDT );
        dllRender( simState, simInput, simGraphics );

        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        glPushMatrix(); {
            const float32_t viewRatio = ( windowHeight + 0.0f ) / ( windowWidth + 0.0f );
            glm::mat4 matWorldView( 1.0f );
            matWorldView *= glm::translate( glm::mat4(1.0f), glm::vec3(-1.0f, -1.0f, 0.0f) );
            matWorldView *= glm::scale( glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 1.0f) );
            matWorldView *= glm::translate( glm::mat4(1.0f), glm::vec3((1.0f-viewRatio)/2.0f, 0.0f, 0.0f) );
            matWorldView *= glm::scale( glm::mat4(1.0f), glm::vec3(viewRatio, 1.0f, 0.0f) );
            glMultMatrixf( &matWorldView[0][0] );

            glEnable( GL_TEXTURE_2D ); {
                // NOTE(JRC): This is required to get the expected/correct texture color,
                // but it's unclear as to why. OpenGL may perform color mixing by default?
                glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
                glBindTexture( GL_TEXTURE_2D, simGraphics->bufferTIDs[hmp::GFX_BUFFER_MASTER] );
                glBegin( GL_QUADS ); {
                    glTexCoord2f( 0.0f, 0.0f ); glVertex2f( 0.0f, 0.0f );
                    glTexCoord2f( 0.0f, 1.0f ); glVertex2f( 0.0f, 1.0f );
                    glTexCoord2f( 1.0f, 1.0f ); glVertex2f( 1.0f, 1.0f );
                    glTexCoord2f( 1.0f, 0.0f ); glVertex2f( 1.0f, 0.0f );
                } glEnd();

                // NOTE(JRC): This subroutine dynamically allocates memory at the simulation level
                // when importing data from OpenGL buffers. This isn't the worst expense, but it
                // could be eliminated by performing a static level of compression to some maximum
                // defined at compile-time.
#ifdef LLCE_CAPTURE
                if( isCapturing ) {
                    LLCE_ALERT_INFO( "Capture Slot {" << currCaptureIdx << "}" );

                    char8_t slotCaptureFileName[csOutputFileNameLength];
                    std::snprintf( &slotCaptureFileName[0],
                        sizeof(slotCaptureFileName),
                        cRenderFileFormat, currCaptureIdx++ );

                    // TODO(JRC): Reversing the colors results in the proper color values,
                    // but it's unclear why this is necessary given that they're stored
                    // internally in the order requested. Debugging may be required in the
                    // future when adapting this code to work on multiple platforms.
                    uicoord32_t textureDims = simGraphics->bufferRess[hmp::GFX_BUFFER_MASTER];
                    color_t* textureData = (color_t*)malloc( sizeof(color_t) * textureDims.x * textureDims.y );
                    glGetTexImage( GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, textureData );

                    // TODO(JRC): If the C 'FILE' strcture isn't cross-platform,
                    // its use needs to be replaced here with something more portable.
                    FILE* textureFile = nullptr;
                    path_t texturePath( 2, cOutputPath.cstr(), slotCaptureFileName );
                    LLCE_ASSERT_ERROR( (textureFile = std::fopen(texturePath, "wb")) != nullptr,
                        "Failed to open render file at path '" << texturePath << "'." );

                    // TODO(JRC): For local memory allocation handling, use png_create_write_struct_2.
                    png_struct* texturePng = nullptr;
                    LLCE_ASSERT_ERROR(
                        (texturePng = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr)) != nullptr,
                        "Failed to create base headers for render file at path '" << texturePath << "'." );
                    png_info* textureInfo = nullptr;
                    LLCE_ASSERT_ERROR(
                        (textureInfo = png_create_info_struct(texturePng)) != nullptr,
                        "Failed to create info headers for render file at path '" << texturePath << "'." );

                    png_init_io( texturePng, textureFile );
                    png_set_IHDR(
                        texturePng, textureInfo, textureDims.x, textureDims.y,
                        8, PNG_COLOR_TYPE_RGBA,
                        PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT );
                    png_write_info( texturePng, textureInfo );
                    // TODO(JRC): Ultimately, it would be best if the data could just
                    // be funneled natively into the PNG interface instead of having
                    // to mirror it about the y-axis.
                    for( uint32_t rowIdx = 0; rowIdx < textureDims.y; rowIdx++ ) {
                        uint32_t rowOff = ( textureDims.y - rowIdx - 1 ) * textureDims.x;
                        png_write_row( texturePng, (png_byte*)&textureData[rowOff] );
                    }
                    png_write_end( texturePng, nullptr );

                    png_destroy_write_struct( &texturePng, &textureInfo );
                    std::fclose( textureFile );
                    free( textureData );
                }
                isCapturing = cIsSimulating;
#endif
                glBindTexture( GL_TEXTURE_2D, 0 );
            } glDisable( GL_TEXTURE_2D );
        } glPopMatrix();

#ifdef LLCE_DEBUG
        glEnable( GL_TEXTURE_2D ); {
            std::snprintf( &textureTexts[cFPSTextureID][0],
                csTextureTextLength,
                "FPS: %0.2f", 1.0 / simDT );
            cGenerateTextTexture( cFPSTextureID, textureColors[cFPSTextureID], textureTexts[cFPSTextureID] );

            glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
            glBindTexture( GL_TEXTURE_2D, textureGLIDs[cFPSTextureID] );
            glBegin( GL_QUADS ); {
                glTexCoord2f( 0.0f, 0.0f ); glVertex2f( -1.0f + 0.0f, -1.0f + 0.2f ); // UL
                glTexCoord2f( 0.0f, 1.0f ); glVertex2f( -1.0f + 0.0f, -1.0f + 0.0f ); // BL
                glTexCoord2f( 1.0f, 1.0f ); glVertex2f( -1.0f + 0.5f, -1.0f + 0.0f ); // BR
                glTexCoord2f( 1.0f, 0.0f ); glVertex2f( -1.0f + 0.5f, -1.0f + 0.2f ); // UR
            } glEnd();

            if( isRecording || isReplaying ) {
                uint32_t textureID = isRecording ? cRecTextureID : cRepTextureID;
                std::snprintf( &textureTexts[textureID][0],
                    csTextureTextLength, isRecording ?
                    "Recording %02d" : "Replaying %02d",
                    recSlotIdx );
                cGenerateTextTexture( textureID, textureColors[textureID], textureTexts[textureID] );

                glBindTexture( GL_TEXTURE_2D, textureGLIDs[textureID] );
                glBegin( GL_QUADS ); {
                    glTexCoord2f( 0.0f, 0.0f ); glVertex2f( -1.0f + 0.0f, +1.0f - 0.0f ); // UL
                    glTexCoord2f( 0.0f, 1.0f ); glVertex2f( -1.0f + 0.0f, +1.0f - 0.2f ); // BL
                    glTexCoord2f( 1.0f, 1.0f ); glVertex2f( -1.0f + 0.5f, +1.0f - 0.2f ); // BR
                    glTexCoord2f( 1.0f, 0.0f ); glVertex2f( -1.0f + 0.5f, +1.0f - 0.0f ); // UR
                } glEnd();

                std::snprintf( &textureTexts[cTimeTextureID][0],
                    csTextureTextLength, isRecording ?
                    "%1u%010u" : "%05u/%05u",
                    repFrameIdx, recFrameCount );
                cGenerateTextTexture( cTimeTextureID, textureColors[textureID], textureTexts[cTimeTextureID] );

                glBindTexture( GL_TEXTURE_2D, textureGLIDs[cTimeTextureID] );
                glBegin( GL_QUADS ); {
                    glTexCoord2f( 0.0f, 0.0f ); glVertex2f( +1.0f - 0.6f, +1.0f - 0.0f ); // UL
                    glTexCoord2f( 0.0f, 1.0f ); glVertex2f( +1.0f - 0.6f, +1.0f - 0.2f ); // BL
                    glTexCoord2f( 1.0f, 1.0f ); glVertex2f( +1.0f - 0.0f, +1.0f - 0.2f ); // BR
                    glTexCoord2f( 1.0f, 0.0f ); glVertex2f( +1.0f - 0.0f, +1.0f - 0.0f ); // UR
                } glEnd();
            }
        } glDisable( GL_TEXTURE_2D );
#endif

        SDL_GL_SwapWindow( window );

        simTimer.split();
        simTimer.wait( cIsSimulating ? 0.0 : -1.0 );
        simDT = simTimer.ft( llce::timer_t::time_e::ideal );
        simFrame += 1;
    }

#ifdef LLCE_CAPTURE
    /// Post-Process Simulation Results ///
    if( cIsSimulating ) {
        // NOTE(JRC): The following code is heavily based on the ffmpeg
        // library's video encoding example:
        // https://ffmpeg.org/doxygen/trunk/encode__video_8c_source.html
        avcodec_register_all();

        const AVCodec* cVideoCodec = avcodec_find_encoder( AV_CODEC_ID_PNG );
        LLCE_ASSERT_ERROR( cVideoCodec != nullptr,
            "Unable to load requested encoding codec in replay capture." );

        AVCodecContext* videoContext = avcodec_alloc_context3( cVideoCodec );
        LLCE_ASSERT_ERROR( videoContext != nullptr,
            "Unable to allocate data for encoding context in replay capture." );

        const uicoord32_t cVideoDims = simGraphics->bufferRess[hmp::GFX_BUFFER_MASTER];
        videoContext->bit_rate = 400000; // TODO(JRC): What should this be?
        videoContext->width = cVideoDims.x;
        videoContext->height = cVideoDims.y;
        videoContext->time_base = (AVRational){1, static_cast<int32_t>(cSimFPS)};
        videoContext->framerate = (AVRational){static_cast<int32_t>(cSimFPS), 1};
        videoContext->gop_size = 0;
        videoContext->pix_fmt = AV_PIX_FMT_RGBA;

        LLCE_ASSERT_ERROR( avcodec_open2(videoContext, cVideoCodec, nullptr) >= 0,
            "Failed to open configured codec in replay capture." );

        FILE* videoFile = nullptr;
        path_t videoPath( 2, cOutputPath.cstr(), cReplayFileFormat );
        LLCE_ASSERT_ERROR( (videoFile = std::fopen(videoPath, "wb")) != nullptr,
            "Failed to open replay file at path '" << videoPath << "'." );

        AVPacket* videoPacket = nullptr;
        LLCE_ASSERT_ERROR( (videoPacket = av_packet_alloc()) != nullptr,
            "Unable to allocate frame packet in replay capture." );
        AVFrame* videoFrame = nullptr;
        LLCE_ASSERT_ERROR( (videoFrame = av_frame_alloc()) != nullptr,
            "Unable to allocate frame in replay capture." );
        videoFrame->format = videoContext->pix_fmt;
        videoFrame->pict_type = AV_PICTURE_TYPE_I;
        videoFrame->width = videoContext->width;
        videoFrame->height = videoContext->height;
        LLCE_ASSERT_ERROR( av_frame_get_buffer(videoFrame, 32) >= 0,
            "Unable to allocate data for frame in replay capture." );

        const auto cEncodeFrame = [ &videoContext, &videoFile ]
                ( uint32_t pFrameIdx, AVFrame* pFrame, AVPacket* pPacket ) {
            LLCE_ASSERT_ERROR( avcodec_send_frame(videoContext, pFrame) >= 0,
                "Unable to send send frame " << pFrameIdx << " for encoding." );

            int32_t encodeStatus = 0;
            while( encodeStatus >= 0 ) {
                encodeStatus = avcodec_receive_packet( videoContext, pPacket );
                if( encodeStatus != AVERROR(EAGAIN) && encodeStatus != AVERROR_EOF ) {
                    LLCE_ASSERT_ERROR( encodeStatus >= 0,
                        "Unable to unpack packet for frame " << pFrameIdx << " for encoding." );
                    std::fwrite( pPacket->data, 1, pPacket->size, videoFile );
                    av_packet_unref( pPacket );
                }
            }
        };

        for( uint32_t frameIdx = 0; frameIdx < recFrameCount; frameIdx++ ) {
            av_frame_make_writable( videoFrame );

            // char8_t frameFileName[csOutputFileNameLength];
            // std::snprintf( &frameFileName[0],
            //     sizeof(frameFileName),
            //     cRenderFileFormat, frameIdx );

            for( uint32_t yIdx = 0; yIdx < videoFrame->height; yIdx++ ) {
                for( uint32_t xIdx = 0; xIdx < videoFrame->width; xIdx++ ) {
                    uint32_t cIdx = yIdx * videoFrame->linesize[0] + xIdx * 4;
                    videoFrame->data[0][cIdx + 0] = 0x00;
                    videoFrame->data[0][cIdx + 1] = 0x2b;
                    videoFrame->data[0][cIdx + 2] = 0x36;
                    videoFrame->data[0][cIdx + 3] = 0xFF;
                }
            }

            videoFrame->pts = frameIdx;

            cEncodeFrame( frameIdx, videoFrame, videoPacket );
        }
        cEncodeFrame( recFrameCount, nullptr, videoPacket );

        const uint8_t cVideoEndcode[] = { 0, 0, 1, 0xb7 };
        std::fwrite( cVideoEndcode, 1, sizeof(cVideoEndcode), videoFile );
        std::fclose( videoFile );

        LLCE_ALERT_INFO( "Replay Capture Slot {" << recSlotIdx << "}" );

        av_frame_free( &videoFrame );
        av_packet_free( &videoPacket );
        avcodec_free_context( &videoContext );
    }
#endif

    /// Clean Up + Exit ///

    if( dllHandle != nullptr ) {
        llce::platform::dllUnloadHandle( dllHandle, cDLLPath );
    }

    recStateStream.close();
    recInputStream.close();

    TTF_CloseFont( font );
    TTF_Quit();

    SDL_GL_DeleteContext( glcontext );
    SDL_DestroyWindow( window );
    SDL_Quit();

    return 0;
}
