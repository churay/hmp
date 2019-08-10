#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_opengl_glext.h>
#include <SDL2/SDL_ttf.h>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <cmath>
#include <cstring>
#include <cstdio>
#include <fstream>

#include "hmp/hmp.h"

#include "timer_t.h"
#include "memory_t.h"
#include "path_t.h"
#include "platform.h"
#include "input.h"
#include "cli.h"
#include "consts.h"

typedef std::ios_base::openmode ioflag_t;
typedef llce::platform::path_t path_t;

typedef const int64_t& (*reduce_f)( const int64_t&, const int64_t& );

typedef bool32_t (*init_f)( hmp::state_t*, hmp::input_t* );
typedef bool32_t (*boot_f)( hmp::output_t* );
typedef bool32_t (*update_f)( hmp::state_t*, hmp::input_t*, const float64_t );
typedef bool32_t (*render_f)( const hmp::state_t*, const hmp::input_t*, const hmp::output_t* );

typedef bool32_t (*kscheck_f)( const llce::input::keyboard_t&, const SDL_Scancode );
typedef uint32_t (*kgcheck_f)( const llce::input::keyboard_t&, const SDL_Scancode*, const uint32_t );

int32_t main( const int32_t pArgCount, const char8_t* pArgs[] ) {
    /// Initialize Global Constant State ///

    const static float64_t csSimFPS = 60.0;

    /// Parse Input Arguments ///

    if( llce::cli::exists("-v", pArgs, pArgCount) ) {
        LLCE_INFO_RELEASE( "{Version: v" << LLCE_VERSION << ", " <<
            "Build: " << (LLCE_DEBUG ? "Debug" : "Release") << ", " <<
            "Libraries: " << (LLCE_DYLOAD ? "Dynamic" : "Static") << ", " <<
            "Floats: " << (LLCE_FDOUBLE ? "Double" : "Single") << "-Precision, " <<
            "Capture*:" << (LLCE_CAPTURE ? "Enabled" : "Disabled") << "}" );
    }

    const char8_t* cSimStateArg = llce::cli::value( "-r", pArgs, pArgCount );
    const int32_t cSimStateIdx = cSimStateArg != nullptr ? std::atoi( cSimStateArg ) : -1;
    const bool32_t cIsSimulating = LLCE_DEBUG ? cSimStateIdx > 0 : false;

    /// Initialize Application Memory/State ///

    // NOTE(JRC): This base address was chosen by following the steps enumerated
    // in the 'doc/static_address.md' documentation file.
    bit8_t* const cBufferAddress = LLCE_DEBUG ? (bit8_t*)0x0000100000000000 : nullptr;
    const uint64_t cBackupBufferCount = LLCE_DEBUG ? static_cast<uint64_t>( 2.0 * csSimFPS ) : 0;
    const uint64_t cSimBufferIdx = 0, cSimBufferLength = MEGABYTE_BL( 1 );
    const uint64_t cBackupBufferIdx = 1, cBackupBufferLength = cBackupBufferCount * cSimBufferLength;
    const uint64_t cBufferLengths[2] = { cSimBufferLength, cBackupBufferLength };

    llce::memory_t mem( 2, &cBufferLengths[0], cBufferAddress );
    hmp::input_t* inputs = (hmp::input_t*)mem.allocate( cSimBufferIdx, 2 * sizeof(hmp::input_t) );

    hmp::state_t* simState = (hmp::state_t*)mem.allocate( cSimBufferIdx, sizeof(hmp::state_t) );
    hmp::input_t* simInput = &inputs[0];
    hmp::output_t* simOutput = (hmp::output_t*)mem.allocate( cSimBufferIdx, sizeof(hmp::output_t) );

#if LLCE_DEBUG
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

    const path_t cAssetPath( 2, cInstallPath.cstr(), "dat" );

    // NOTE(JRC): It's important to realize that the pathing here ties debugging
    // outputs to particular builds of the code. This is probably a good thing, but
    // it does mean clearing the current build will cause all debug files to be lost.
    const path_t cOutputPath( 2, cInstallPath.cstr(), "out" );
    const char8_t* cStateFileFormat = "state%u.dat";
    const char8_t* cInputFileFormat = "input%u.dat";
    const char8_t* cRenderFileFormat = "render%u-%u.png";
    const static int32_t csOutputFileNameLength = 20;

    /// Load Dynamic Shared Libraries ///

    const static char8_t* csDLLFileNames[] = { "libhmpdata.so", "libhmp.so" };
    const static uint32_t csDLLCount = ARRAY_LEN( csDLLFileNames );
    const uint32_t cDataDLLID = 0, cCodeDLLID = 1;

    path_t dllFilePaths[csDLLCount];
    void* dllHandles[csDLLCount];
#if LLCE_DYLOAD
    for( uint32_t dllIdx = 0; dllIdx < csDLLCount; dllIdx++ ) {
        const char8_t* cDLLFileName = csDLLFileNames[dllIdx];
        path_t& dllFilePath = dllFilePaths[dllIdx];

        dllFilePath = llce::platform::libFindDLLPath( cDLLFileName );
        LLCE_ASSERT_ERROR( dllFilePath.exists(),
            "Failed to find library '" << cDLLFileName << "' in dynamic path." );
    }
#endif

    init_f dllInit = nullptr;
    boot_f dllBoot = nullptr;
    update_f dllUpdate = nullptr;
    render_f dllRender = nullptr;
    const auto cDLLReload = [ &dllFilePaths, &dllHandles, &dllInit, &dllBoot, &dllUpdate, &dllRender ] () {
#if !LLCE_DYLOAD
        dllInit = &init;
        dllBoot = &boot;
        dllUpdate = &update;
        dllRender = &render;

        return true;
#else
        bool32_t dllLoadSuccess = true;

        // NOTE(JRC): This needs to happen in two stages to ensure that libraries
        // with dynamic dependencies fully reload new symbol values during this process.
        for( uint32_t dllIdx = 0; dllIdx < csDLLCount && dllLoadSuccess; dllIdx++ ) {
            void*& dllHandle = dllHandles[dllIdx];
            if( dllHandle != nullptr ) {
                llce::platform::dllUnloadHandle( dllHandle, dllFilePaths[dllIdx] );
            }
        } for( uint32_t dllIdx = 0; dllIdx < csDLLCount && dllLoadSuccess; dllIdx++ ) {
            void*& dllHandle = dllHandles[dllIdx];
            dllHandle = llce::platform::dllLoadHandle( dllFilePaths[dllIdx] );
            dllLoadSuccess &= dllHandle != nullptr;
        }

        if( dllLoadSuccess ) {
            void* dllHandle = dllHandles[cCodeDLLID];
            dllInit = (init_f)llce::platform::dllLoadSymbol( dllHandle, "init" );
            dllBoot = (boot_f)llce::platform::dllLoadSymbol( dllHandle, "boot" );
            dllUpdate = (update_f)llce::platform::dllLoadSymbol( dllHandle, "update" );
            dllRender = (render_f)llce::platform::dllLoadSymbol( dllHandle, "render" );
        }

        return dllLoadSuccess &&
            dllInit != nullptr && dllBoot != nullptr &&
            dllUpdate != nullptr && dllRender != nullptr;
#endif
    };

    // NOTE(JRC): Reduces all DLL modification times to a single value based
    // on the given function (e.g. std::min for earliest, std::max for latest).
    const auto cDLLModTime = [ &dllFilePaths ] ( reduce_f pReduce ) {
#if !LLCE_DYLOAD
        return 1;
#else
        int64_t reducedModTime = -1;
        for( uint32_t dllIdx = 0; dllIdx < csDLLCount; dllIdx++ ) {
            int64_t dllModTime = dllFilePaths[dllIdx].modtime();
            reducedModTime = ( reducedModTime < 0 ) ?
                dllModTime : pReduce( dllModTime, reducedModTime );
        }
        return reducedModTime;
#endif
    };

    LLCE_ASSERT_ERROR( cDLLReload(),
        "Couldn't load dynamic library symbols on initialize." );

    int64_t prevDylibModTime, currDylibModTime;
    LLCE_ASSERT_ERROR(
        prevDylibModTime = currDylibModTime = cDLLModTime(std::min<int64_t>),
        "Couldn't load dynamic library stat data on initialize." );

    /// Initialize Windows/Graphics ///

    // TODO(JRC): Include 'SDL_INIT_GAMECONTROLLER' when it's needed; it causes
    // extra one-time leaks so it has been excluded to aid in memory error tracking.
    LLCE_ASSERT_ERROR(
        SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) >= 0,
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

    { /// Initialize Window ///
        const path_t cIconPath( 2, cAssetPath.cstr(), "icon.png" );
        static color4u8_t sIconBuffer[LLCE_MAX_RESOLUTION];

        uint32_t iconWidth = 0, iconHeight = 0;
        LLCE_VERIFY_WARNING(
            llce::platform::pngLoad(cIconPath, (bit8_t*)&sIconBuffer[0], iconWidth, iconHeight),
            "Failed to load icon at path '" << cIconPath << "'." );

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        const uint32_t cRMask = 0xFF000000;
        const uint32_t cGMask = 0x00FF0000;
        const uint32_t cBMask = 0x0000FF00;
        const uint32_t cAMask = 0x000000FF;
#else // little endian, like x86
        const uint32_t cRMask = 0x000000FF;
        const uint32_t cGMask = 0x0000FF00;
        const uint32_t cBMask = 0x00FF0000;
        const uint32_t cAMask = 0xFF000000;
#endif

        SDL_Surface* windowIcon = nullptr;
        windowIcon = SDL_CreateRGBSurfaceFrom( (bit8_t*)&sIconBuffer[0], iconWidth, iconHeight,
            sizeof(color4u8_t) * 8, sizeof(color4u8_t) * iconWidth, cRMask, cGMask, cBMask, cAMask );
        LLCE_CHECK_WARNING( windowIcon != nullptr,
            "Failed to load the icon for the application." );

        SDL_SetWindowIcon( window, windowIcon );
        SDL_FreeSurface( windowIcon );
    }

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

    /// Initialize Audio ///

    const static uint32_t csAudioFrequency = 48000;                         // audio samples / second
    const static SDL_AudioFormat csAudioFormat = AUDIO_S16LSB;              // audio sample data format
    const static uint32_t csAudioChannelCount = 2;                          // audio channels (2: stereo)
    const static uint32_t csAudioSampleBytes = 2 * csAudioChannelCount;     // audio bytes / sample
    const static uint32_t csAudioSampleCount = ( csAudioFrequency * csAudioSampleBytes ) / csSimFPS;
    int16_t audioBuffer[csAudioSampleCount * csAudioChannelCount];          // audio frame buffer

    SDL_AudioSpec tempAudioConfig = {0}; {
        tempAudioConfig.freq = csAudioFrequency;
        tempAudioConfig.format = csAudioFormat;
        tempAudioConfig.channels = csAudioChannelCount;
        tempAudioConfig.samples = csAudioSampleCount;
        tempAudioConfig.callback = nullptr;
    }
    const SDL_AudioSpec wantAudioConfig = tempAudioConfig;
    SDL_AudioSpec realAudioConfig;

    SDL_AudioDeviceID audioDeviceID = 1;
    LLCE_ASSERT_ERROR(
        (audioDeviceID = SDL_OpenAudioDevice(
            nullptr, 0, &tempAudioConfig, &realAudioConfig, SDL_AUDIO_ALLOW_ANY_CHANGE)) >= 0,
        "SDL failed to initialize audio device; " << SDL_GetError() );
    LLCE_ASSERT_ERROR(
        wantAudioConfig.channels == realAudioConfig.channels && wantAudioConfig.format == realAudioConfig.format,
        "SDL failed to initialize audio device with correct format." );

    SDL_PauseAudioDevice( audioDeviceID, false );

    /// Generate Graphics Assets ///

    const char8_t* cFontFileName = "dejavu_mono.ttf";
    const path_t cFontPath( 2, cAssetPath.cstr(), cFontFileName );
    LLCE_ASSERT_ERROR( cFontPath.exists(),
        "Failed to locate font with file name '" << cFontFileName << "'." );

    const int32_t cFontSize = 20;
    TTF_Font* font = TTF_OpenFont( cFontPath, cFontSize );
    LLCE_ASSERT_ERROR( font != nullptr,
        "SDL-TTF failed to create font; " << TTF_GetError() );

    const static color4u8_t csBlackColor = { 0x00, 0x00, 0x00, 0x00 };
    const static color4u8_t csWhiteColor = { 0xFF, 0xFF, 0xFF, 0xFF };

#if LLCE_DEBUG
    const static uint32_t csTextureTextLength = 20;
    uint32_t textureGLIDs[] = { 0, 0, 0, 0 };
    color4u8_t textureColors[] = { {0xFF, 0x00, 0x00, 0xFF}, {0x00, 0xFF, 0x00, 0xFF}, {0x00, 0x00, 0xFF, 0xFF} };
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
            ( const uint32_t pTextureID, const color4u8_t pTextureColor, const char8_t* pTextureText ) {
        const uint32_t& textureGLID = textureGLIDs[pTextureID];

        SDL_Color renderColor = { pTextureColor.x, pTextureColor.y, pTextureColor.z, pTextureColor.w };
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

#if LLCE_CAPTURE
    static color4u8_t sCaptureBuffer[LLCE_MAX_RESOLUTION];
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

    kscheck_f isKeyDown = llce::input::isKeyDown;
    kgcheck_f isKGDown = llce::input::isKGDown;
    kscheck_f isKeyPressed = llce::input::isKeyPressed;
    kgcheck_f isKGPressed = llce::input::isKGPressed;
    kscheck_f isKeyReleased = llce::input::isKeyReleased;
    kgcheck_f isKGReleased = llce::input::isKGReleased;

    /// Update/Render Loop ///

    bool32_t isRunning = true, isStepping = false;

    bool32_t isRecording = false, isReplaying = false;
    uint32_t currSlotIdx = 0, recSlotIdx = 0;
    uint32_t repFrameIdx = 0, recFrameCount = 0;

#if LLCE_CAPTURE
    bool32_t isCapturing = cIsSimulating;
#else
    bool32_t isCapturing = false;
#endif
    uint32_t currCaptureIdx = 0;

    llce::timer_t simTimer( csSimFPS, llce::timer_t::ratio_e::fps );
    float64_t simDT = 0.0;
    // NOTE(JRC): A cursory check shows that it will take ~1e10 years of
    // uninterrupted run time for this to overflow at 60 FPS, so the fact
    // that this increments very quickly over time isn't a big concern.
    uint64_t simFrame = 0;

    isRunning &= dllInit( simState, simInput );
    isRunning &= dllBoot( simOutput );

    // TODO(JRC): This isn't a great solution and it should be improved
    // if at all possible to be less 'raw'.
    simOutput->sfxConfig = realAudioConfig;
    simOutput->sfxBuffers[hmp::SFX_BUFFER_MASTER] = (bit8_t*)&audioBuffer[0];

    while( isRunning ) {
#if LLCE_DEBUG
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

        llce::input::readKeyboard( appInput->keyboard );
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

        if( isKeyDown(appInput->keyboard, SDL_SCANCODE_Q) ) {
            // q key = quit application
            isRunning = false;
        } if( isKeyPressed(appInput->keyboard, SDL_SCANCODE_GRAVE) ) {
            // ` key = capture application
            isCapturing = true;
        }
#if LLCE_DEBUG
        uint64_t backupIdx = simFrame % cBackupBufferCount;
        std::memcpy( (void*)&backupInputs[backupIdx], (void*)appInput, sizeof(hmp::input_t) );
        std::memcpy( (void*)&backupStates[backupIdx], (void*)simState, sizeof(hmp::state_t) );

        if( (!isStepping && isKeyPressed(appInput->keyboard, SDL_SCANCODE_SPACE)) ||
                (isStepping && isKeyDown(appInput->keyboard, SDL_SCANCODE_SPACE)) ) {
            // space key = toggle frame advance mode
            LLCE_INFO_DEBUG( "Frame Advance <" << (!isStepping ? "ON " : "OFF") << ">" );
            isStepping = !isStepping;
        }

        if( (currSlotIdx = isKGPressed(appInput->keyboard, &cFXKeyGroup[0], cFXKeyGroupSize)) || (cIsSimulating && !isReplaying) ) {
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

            if( (isKeyDown(appInput->keyboard, SDL_SCANCODE_LSHIFT) && !isRecording) || cIsSimulating ) {
                // lshift + fx = toggle slot x replay
                LLCE_INFO_DEBUG( "Replay Slot {" << recSlotIdx << "} <" << (!isReplaying ? "ON " : "OFF") << ">" );
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
            } else if( isKeyDown(appInput->keyboard, SDL_SCANCODE_RSHIFT) && !isRecording ) {
                // rshift + fx = hotload slot x state (reset replay)
                LLCE_INFO_DEBUG( "Hotload Slot {" << recSlotIdx << "}" );
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
                LLCE_INFO_DEBUG( "Record Slot {" << recSlotIdx << "} <" << (!isRecording ? "ON " : "OFF") << ">" );
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
                LLCE_INFO_DEBUG( "Hotsave Slot {" << recSlotIdx << "}" );
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
            currDylibModTime = cDLLModTime(std::max<int64_t>),
            "Couldn't load dynamic library stat data on step." );
        if( currDylibModTime != prevDylibModTime ) {
            // TODO(JRC): This isn't ideal since spinning in this way can really
            // ramp up processing time, but it's a permissible while there aren't
            // too many iterations per reload (there are none at time of writing).
            // TODO(JRC): Consider clearing out the audio queue at this point
            // because the hot-loaded state could lag as a result of existing audio.
            uint32_t lockSpinCount = 0;
            while( cInstallLockPath.exists() ) { lockSpinCount++; }

            LLCE_CHECK_WARNING( lockSpinCount == 0,
                "Performed " << lockSpinCount << " spin cycles " <<
                "while waiting for DLL install; consider transitioning to file locks." );

            LLCE_VERIFY_ERROR( cDLLReload(),
                "Couldn't load dynamic library symbols at " <<
                "simulation time " << simTimer.tt() << "." );

            LLCE_INFO_DEBUG( "DLL Reload {" << simFrame << "}" );

            prevDylibModTime = currDylibModTime;
        }
#endif
        glViewport( 0, 0, windowWidth, windowHeight );

        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();
        glOrtho( -1.0f, +1.0f, -1.0f, +1.0f, -1.0f, +1.0f );
        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity();

        std::memset( audioBuffer, 0, sizeof(audioBuffer) );

        isRunning &= dllUpdate( simState, simInput, simDT );
        isRunning &= dllRender( simState, simInput, simOutput );

        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        glPushMatrix(); {
            const float32_t viewRatio = ( windowHeight + 0.0f ) / ( windowWidth + 0.0f );
            glm::mat4 matWorldView( 1.0f );
            matWorldView *= glm::translate( glm::mat4(1.0f), glm::vec3(-1.0f, -1.0f, 0.0f) );
            matWorldView *= glm::scale( glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 1.0f) );
            matWorldView *= glm::translate( glm::mat4(1.0f), glm::vec3((1.0f-viewRatio)/2.0f, 0.0f, 0.0f) );
            matWorldView *= glm::scale( glm::mat4(1.0f), glm::vec3(viewRatio, 1.0f, 1.0f) );
            glMultMatrixf( &matWorldView[0][0] );

            glEnable( GL_TEXTURE_2D ); {
                // NOTE(JRC): This is required to get the expected/correct texture color,
                // but it's unclear as to why. OpenGL may perform color mixing by default?
                glColor4ubv( (uint8_t*)&csWhiteColor );
                glBindTexture( GL_TEXTURE_2D, simOutput->gfxBufferCBOs[hmp::GFX_BUFFER_MASTER] );
                glBegin( GL_QUADS ); {
                    glTexCoord2f( 0.0f, 0.0f ); glVertex2f( 0.0f, 0.0f );
                    glTexCoord2f( 0.0f, 1.0f ); glVertex2f( 0.0f, 1.0f );
                    glTexCoord2f( 1.0f, 1.0f ); glVertex2f( 1.0f, 1.0f );
                    glTexCoord2f( 1.0f, 0.0f ); glVertex2f( 1.0f, 0.0f );
                } glEnd();

#if LLCE_CAPTURE
                if( isCapturing ) {
                    LLCE_INFO_RELEASE( "Capture Slot {" << recSlotIdx << "-" << currCaptureIdx << "}" );

                    char8_t slotCaptureFileName[csOutputFileNameLength];
                    std::snprintf( &slotCaptureFileName[0],
                        sizeof(slotCaptureFileName),
                        cRenderFileFormat, recSlotIdx, currCaptureIdx++ );
                    path_t capturePath( 2, cOutputPath.cstr(), slotCaptureFileName );

                    // TODO(JRC): Reversing the colors results in the proper color values,
                    // but it's unclear why this is necessary given that they're stored
                    // internally in the order requested. Debugging may be required in the
                    // future when adapting this code to work on multiple platforms.
                    vec2u32_t captureDims = simOutput->gfxBufferRess[hmp::GFX_BUFFER_MASTER];
                    glGetTexImage( GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, sCaptureBuffer );

                    // TODO(JRC): Ultimately, it would be best if the data could just
                    // be funneled natively into the PNG interface instead of having
                    // to mirror it about the y-axis.
                    color4u8_t* tempBuffer = &sCaptureBuffer[captureDims.x * captureDims.y];
                    uint32_t bufferByteCount = captureDims.x * sizeof( color4u8_t );
                    for( uint32_t rowIdx = 0; rowIdx < captureDims.y / 2; rowIdx++ ) {
                        uint32_t rowOff = rowIdx * captureDims.x;
                        uint32_t oppOff = ( captureDims.y - rowIdx - 1 ) * captureDims.x;
                        std::memcpy( tempBuffer, &sCaptureBuffer[rowOff], bufferByteCount );
                        std::memcpy( &sCaptureBuffer[rowOff], &sCaptureBuffer[oppOff], bufferByteCount );
                        std::memcpy( &sCaptureBuffer[oppOff], tempBuffer, bufferByteCount );
                    }

                    LLCE_VERIFY_WARNING(
                        llce::platform::pngSave(capturePath, (bit8_t*)&sCaptureBuffer[0], captureDims.x, captureDims.y),
                        "Failed to capture frame {" << simFrame << "} to path '" << capturePath << "'." );
                }
                isCapturing = cIsSimulating;
#endif
                glBindTexture( GL_TEXTURE_2D, 0 );
            } glDisable( GL_TEXTURE_2D );
        } glPopMatrix();

#if LLCE_DEBUG
        glEnable( GL_TEXTURE_2D ); {
            std::snprintf( &textureTexts[cFPSTextureID][0],
                csTextureTextLength,
                "FPS: %0.2f", 1.0 / simDT );
            cGenerateTextTexture( cFPSTextureID, textureColors[cFPSTextureID], textureTexts[cFPSTextureID] );

            glColor4ubv( (uint8_t*)&csWhiteColor );
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

        // TODO(JRC): Improve accounting for currently buffered sound data. There
        // shouldn't be any since we fill the entire buffer every frame, but lag
        // and backfill may occur in high memory/compute load situations.
        if( simOutput->sfxDirtyBits[hmp::SFX_BUFFER_MASTER] && !cIsSimulating ) {
            SDL_QueueAudio( audioDeviceID, &audioBuffer[0], sizeof(audioBuffer) );
            simOutput->sfxDirtyBits[hmp::SFX_BUFFER_MASTER] = false;
        }

        SDL_GL_SwapWindow( window );

        simTimer.split();
        simTimer.wait( cIsSimulating ? 0.0 : -1.0 );
        simDT = simTimer.ft( llce::timer_t::time_e::ideal );
        simFrame += 1;
    }

    /// Clean Up + Exit ///

#if LLCE_DYLOAD
    for( uint32_t dllIdx = 0; dllIdx < csDLLCount; dllIdx++ ) {
        if( dllHandles[dllIdx] != nullptr ) {
            llce::platform::dllUnloadHandle( dllHandles[dllIdx], dllFilePaths[dllIdx] );
        }
    }
#endif

    recStateStream.close();
    recInputStream.close();

    TTF_CloseFont( font );
    TTF_Quit();

    SDL_CloseAudioDevice( audioDeviceID );

    SDL_GL_DeleteContext( glcontext );
    SDL_DestroyWindow( window );
    SDL_Quit();

    return 0;
}
