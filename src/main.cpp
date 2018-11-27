#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_ttf.h>

#include <cstring>
#include <iostream>
#include <fstream>

#include "hmp/hmp.h"
#include "hmp/hmp_box_t.h"

#include "timer_t.h"
#include "memory_t.h"
#include "path_t.h"
#include "platform.h"
#include "util.h"
#include "consts.h"

typedef void (*init_f)( hmp::state_t*, hmp::input_t* );
typedef void (*update_f)( hmp::state_t*, hmp::input_t*, const float64_t );
typedef void (*render_f)( const hmp::state_t*, const hmp::input_t* );
typedef std::ios_base::openmode ioflag_t;
typedef llce::platform::path_t path_t;
typedef llce::util::color_t color_t;

int main() {
    /// Initialize Application Memory/State ///

    // NOTE(JRC): This base address was chosen by following the steps enumerated
    // in the 'doc/static_address.md' documentation file.
#ifdef LLCE_DEBUG
    bit8_t* const cBufferAddress = (bit8_t*)0x0000100000000000;
#else
    bit8_t* const cBufferAddress = nullptr;
#endif
    const uint64_t cBufferLength = MEGABYTE_BL( 1 );
    llce::memory_t mem( 1, &cBufferLength, cBufferAddress );
    hmp::input_t rawInput;

    std::fstream recStateStream, recInputStream;
    const ioflag_t cIOModeR = std::fstream::binary | std::fstream::in;
    const ioflag_t cIOModeW = std::fstream::binary | std::fstream::out | std::fstream::trunc;

    hmp::state_t* state = (hmp::state_t*)mem.allocate( 0, sizeof(hmp::state_t) );
    hmp::input_t* input = &rawInput;

    /// Find Project Paths ///

    const path_t cExePath = llce::platform::exeBasePath();
    LLCE_ASSERT_ERROR( cExePath.exists(),
        "Failed to find path to running executable." );

    const path_t cInstallPath( 2, cExePath.cstr(), path_t::DUP );
    LLCE_ASSERT_ERROR( cInstallPath.exists(),
        "Failed to find path to running executable." );
    const path_t cInstallLockPath( 2, cInstallPath.cstr(), "install.lock" );

    const path_t cStateFilePath( 3, cInstallPath.cstr(), "out", "state.dat" );
    const path_t cInputFilePath( 3, cInstallPath.cstr(), "out", "input.dat" );

    /// Load Dynamic Shared Library ///

    const char8_t* cDLLFileName = "libhmp.so";
    const path_t cDLLPath = llce::platform::libFindDLLPath( cDLLFileName );
    LLCE_ASSERT_ERROR( cDLLPath.exists(),
        "Failed to find library " << cDLLFileName << " in dynamic path." );

    void* dllHandle = nullptr;
    init_f dllInit = nullptr;
    update_f dllUpdate = nullptr;
    render_f dllRender = nullptr;
    const auto cDLLReload = [ &cDLLPath, &dllHandle, &dllInit, &dllUpdate, &dllRender ] () {
        if( dllHandle != nullptr ) {
            llce::platform::dllUnloadHandle( dllHandle, cDLLPath );
        }

        dllHandle = llce::platform::dllLoadHandle( cDLLPath );
        dllInit = (init_f)llce::platform::dllLoadSymbol( dllHandle, "init" );
        dllUpdate = (update_f)llce::platform::dllLoadSymbol( dllHandle, "update" );
        dllRender = (render_f)llce::platform::dllLoadSymbol( dllHandle, "render" );

        return dllHandle != nullptr &&
            dllInit != nullptr && dllUpdate != nullptr && dllRender != nullptr;
    };

    LLCE_ASSERT_ERROR( cDLLReload(),
        "Couldn't load library `" << cDLLFileName << "` symbols on initialize." );

    int64_t prevDylibModTime, currDylibModTime;
    LLCE_ASSERT_ERROR(
        prevDylibModTime = currDylibModTime = cDLLPath.modtime(),
        "Couldn't load library `" << cDLLFileName << "` stat data on initialize." );

    /// Initialize Windows/Graphics ///

    LLCE_ASSERT_ERROR(
        SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) >= 0,
        "SDL failed to initialize; " << SDL_GetError() );

    LLCE_ASSERT_ERROR(
        TTF_Init() >= 0,
        "SDL-TTF failed to initialize; " << TTF_GetError() );

    { // Initialize OpenGL Context //
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2 );

        SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 ); // double-buffer
        SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 32 );

        SDL_GL_SetSwapInterval( 1 ); //vsync
    }

    int32_t windowWidth = 640, windowHeight = 480;
    SDL_Window* window = SDL_CreateWindow(
        "Loop-Live Code Editing",                   // Window Title
        SDL_WINDOWPOS_UNDEFINED,                    // Window X Position
        SDL_WINDOWPOS_UNDEFINED,                    // Window Y Position
        windowWidth,                                // Window Width
        windowHeight,                               // Window Height
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE ); // Window Flags
    LLCE_ASSERT_ERROR( window != nullptr,
        "SDL failed to create window instance; " << SDL_GetError() );

    SDL_GLContext glcontext = SDL_GL_CreateContext( window );
    LLCE_ASSERT_ERROR( glcontext != nullptr,
        "SDL failed to generate OpenGL context; " << SDL_GetError() );

    { // Configure OpenGL Context //
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        glEnable( GL_BLEND );
        glDisable( GL_LIGHTING );
        glEnable( GL_TEXTURE_2D );
        glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
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
    const static uint32_t csTextureTextCap = 20;
    uint32_t textureGLIDs[] = { 0, 0, 0, 0 };
    color_t textureColors[] = { {0xFF, 0x00, 0x00, 0xFF}, {0x00, 0xFF, 0x00, 0xFF}, {0x00, 0x00, 0xFF, 0xFF} };
    char8_t textureTexts[][csTextureTextCap] = { "FPS: ???", "Recording", "Replaying", "Time: ???" };
    const uint32_t cFPSTextureID = 0, cRecTextureID = 1, cRepTextureID = 2, cTimeTextureID = 3;

    const uint32_t cTextureCount = sizeof( textureGLIDs ) / sizeof( textureGLIDs[0] );
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
    // is stored in memory. Since the texture buffers are all statically sizes, it
    // would be ideal if the same, statically allocated arrays were filled in this
    // method, but customizing memory allocations for SDL isn't easy to do. For a
    // performance-level texture generation method, watch the "Handmade Hero" tutorials
    // on OpenGL texturing and font APIs.
    const auto cGenerateTextTexture = [ &textureGLIDs, &font ]
            ( const uint32_t textureID, const color_t textureColor, const char8_t* textureText ) {
        const uint32_t& textureGLID = textureGLIDs[textureID];

        SDL_Color renderColor = { textureColor.r, textureColor.g, textureColor.b, textureColor.a };
        SDL_Surface* textSurface = TTF_RenderText_Solid( font, textureText, renderColor );
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

    const auto cIsKeyDown = [ &input ] ( const SDL_Scancode pKey ) {
        return (bool32_t)( input->keys[pKey] );
    };

    const auto cWasKeyPressed = [ &input ] ( const SDL_Scancode pKey ) {
        return (bool32_t)( input->keys[pKey] && input->diffs[pKey] == hmp::KEY_DIFF_DOWN );
    };

    const auto cWasKeyReleased = [ &input ] ( const SDL_Scancode pKey ) {
        return (bool32_t)( input->keys[pKey] && input->diffs[pKey] == hmp::KEY_DIFF_UP );
    };

    /// Update/Render Loop ///

    bool32_t isRunning = true;
    bool32_t isRecording = false, isReplaying = false;
    uint32_t repFrameIdx = 0, recFrameCount = 0;

    llce::timer_t simTimer( 60, llce::timer_t::type::fps );
    float64_t simDT = 0.0;

    dllInit( state, input );
    while( isRunning ) {
        simTimer.split();

        const uint8_t* keyboardState = SDL_GetKeyboardState( nullptr );
        std::memcpy( input->keys, keyboardState, sizeof(input->keys) );
        std::memset( input->diffs, hmp::KEY_DIFF_NONE, sizeof(input->diffs) );

        SDL_Event event;
        while( SDL_PollEvent(&event) ) {
            if( event.type == SDL_QUIT ) {
                isRunning = false;
            } else if( event.type == SDL_WINDOWEVENT  && (
                   event.window.event == SDL_WINDOWEVENT_RESIZED ||
                   event.window.event == SDL_WINDOWEVENT_EXPOSED) ) {
                SDL_GetWindowSize( window, &windowWidth, &windowHeight );
            } else if( event.type == SDL_KEYDOWN ) {
                input->keys[event.key.keysym.scancode] = true;
                input->diffs[event.key.keysym.scancode] = hmp::KEY_DIFF_DOWN;
            } else if( event.type == SDL_KEYUP ) {
                input->keys[event.key.keysym.scancode] = false;
                input->diffs[event.key.keysym.scancode] = hmp::KEY_DIFF_UP;
            }
        }

#ifdef LLCE_DEBUG
        if( cIsKeyDown(SDL_SCANCODE_Q) ) {
            isRunning = false;
        } else if( cWasKeyPressed(SDL_SCANCODE_SPACE) ) {
            std::memset( (void*)state, 0, sizeof(hmp::state_t) );
            dllInit( state, input );
        } else if( cWasKeyPressed(SDL_SCANCODE_T) && !isRecording ) {
            if( !isReplaying ) {
                repFrameIdx = 0;
                recStateStream.open( cStateFilePath, cIOModeR );
                recInputStream.open( cInputFilePath, cIOModeR );

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
        } else if( cWasKeyPressed(SDL_SCANCODE_R) && !isReplaying ) {
            if( !isRecording ) {
                recFrameCount = 0;
                recStateStream.open( cStateFilePath, cIOModeW );
                recStateStream.write( mem.buffer(), mem.length() );
                recStateStream.close();
                recInputStream.open( cInputFilePath, cIOModeW );
            } else {
                recInputStream.close();
            }
            isRecording = !isRecording;
        }

        // TODO(JRC): This is a bit weird for replaying because we allow intercepts
        // from any key before replacing all key presses with replay data. This is
        // good in some ways as it allows recordings to be excited, but it does
        // open the door for weird behavior like embedded recordings.
        if( isRecording ) {
            recInputStream.write( (bit8_t*)input, sizeof(hmp::input_t) );
            recFrameCount++;
        } if( isReplaying ) {
            if( recInputStream.peek() == EOF || recInputStream.eof() ) {
                repFrameIdx = 0;
                recStateStream.seekg( 0 );
                recStateStream.read( mem.buffer(), mem.length() );
                recInputStream.seekg( 0 );
            }
            recInputStream.read( (bit8_t*)input, sizeof(hmp::input_t) );
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

            prevDylibModTime = currDylibModTime;
        }
#endif

        glViewport( 0, 0, windowWidth, windowHeight );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();
        glOrtho( -1.0f, +1.0f, -1.0f, +1.0f, -1.0f, +1.0f );
        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity();

        glPushMatrix(); {
            const float32_t viewRatio = ( windowHeight + 0.0f ) / ( windowWidth + 0.0f );

            glm::mat4 matWorldView( 1.0f );
            matWorldView *= glm::translate( glm::mat4(1.0f), glm::vec3(-1.0f, -1.0f, 0.0f) );
            matWorldView *= glm::scale( glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 1.0f) );
            matWorldView *= glm::translate( glm::mat4(1.0f), glm::vec3((1.0f-viewRatio)/2.0f, 0.0f, 0.0f) );
            matWorldView *= glm::scale( glm::mat4(1.0f), glm::vec3(viewRatio, 1.0f, 0.0f) );
            glMultMatrixf( &matWorldView[0][0] );

            glBegin( GL_QUADS ); {
                glColor4ub( 0x00, 0x2b, 0x36, 0xFF );
                glVertex2f( 0.0f, 0.0f );
                glVertex2f( 1.0f, 0.0f );
                glVertex2f( 1.0f, 1.0f );
                glVertex2f( 0.0f, 1.0f );
            } glEnd();

            dllUpdate( state, input, simDT );
            dllRender( state, input );
        } glPopMatrix();

#ifdef LLCE_DEBUG
        glEnable( GL_TEXTURE_2D ); {
            std::snprintf( &textureTexts[cFPSTextureID][0],
                csTextureTextCap,
                "FPS: %0.2f", simTimer.fps() );
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

                glBindTexture( GL_TEXTURE_2D, textureGLIDs[textureID] );
                glBegin( GL_QUADS ); {
                    glTexCoord2f( 0.0f, 0.0f ); glVertex2f( -1.0f + 0.0f, +1.0f - 0.0f ); // UL
                    glTexCoord2f( 0.0f, 1.0f ); glVertex2f( -1.0f + 0.0f, +1.0f - 0.2f ); // BL
                    glTexCoord2f( 1.0f, 1.0f ); glVertex2f( -1.0f + 0.5f, +1.0f - 0.2f ); // BR
                    glTexCoord2f( 1.0f, 0.0f ); glVertex2f( -1.0f + 0.5f, +1.0f - 0.0f ); // UR
                } glEnd();

                std::snprintf( &textureTexts[cTimeTextureID][0],
                    csTextureTextCap, isRecording ?
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

        simTimer.split( true );
        simDT = simTimer.ft();
    }

    /// Clean Up + Exit ///

    recStateStream.close();
    recInputStream.close();

    TTF_CloseFont( font );
    TTF_Quit();

    SDL_DestroyWindow( window );
    SDL_Quit();

    return 0;
}
