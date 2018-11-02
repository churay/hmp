#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_ttf.h>

#include <cstring>
#include <iostream>
#include <fstream>

#include "hmp/hmp.h"

#include "timer_t.h"
#include "memory_t.h"
#include "path_t.h"
#include "platform.h"
#include "consts.h"

typedef void (*update_f)( hmp::state*, hmp::input* );
typedef void (*render_f)( const hmp::state*, const hmp::input* );
typedef std::ios_base::openmode ioflag_t;
typedef llce::platform::path_t path_t;

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

    hmp::state* state = (hmp::state*)mem.allocate( 0, sizeof(hmp::state) ); {
        hmp::state temp;
        memcpy( state, &temp, sizeof(hmp::state) );

        state->box[2] = 0.1;
        state->box[3] = 0.1;
        state->box[0] = 0.0 - state->box[2] / 2.0;
        state->box[1] = 0.0 - state->box[3] / 2.0;
    }

    std::fstream recStateStream, recInputStream;
    const ioflag_t cIOModeR = std::fstream::binary | std::fstream::in;
    const ioflag_t cIOModeW = std::fstream::binary | std::fstream::out | std::fstream::trunc;

    /// Initialize Input State ///

    hmp::input rawInput;
    hmp::input* input = &rawInput;

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

    void* hmpLibHandle = llce::platform::dllLoadHandle( cDLLPath );
    void* updateSymbol = llce::platform::dllLoadSymbol( hmpLibHandle, "update" );
    void* renderSymbol = llce::platform::dllLoadSymbol( hmpLibHandle, "render" );
    LLCE_ASSERT_ERROR(
        hmpLibHandle != nullptr && updateSymbol != nullptr && renderSymbol != nullptr,
        "Couldn't load library `" << cDLLFileName << "` symbols on initialize." );

    update_f updateFunction = (update_f)updateSymbol;
    render_f renderFunction = (render_f)renderSymbol;

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
    uint32_t textureColors[] = { 0xFF0000FF, 0xFF00FF00, 0xFFFF0000 }; // little endian
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
            ( const uint32_t textureID, const uint32_t textureColor, const char8_t* textureText ) {
        const uint32_t& textureGLID = textureGLIDs[textureID];

        SDL_Color renderColor = {
            (uint8_t)( (textureColor >> 0*8) & 0xFF ),
            (uint8_t)( (textureColor >> 1*8) & 0xFF ),
            (uint8_t)( (textureColor >> 2*8) & 0xFF ),
            (uint8_t)( (textureColor >> 3*8) & 0xFF ) };

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

    /// Update/Render Loop ///

    bool32_t isRunning = true;
    bool32_t isRecording = false, isReplaying = false;
    uint32_t repFrameIdx = 0, recFrameCount = 0;

    llce::timer_t simTimer( 60, llce::timer_t::type::fps );

    while( isRunning ) {
        simTimer.split();

        const uint8_t* keyboardState = SDL_GetKeyboardState( nullptr );
        std::memcpy( input->keys, keyboardState, sizeof(input->keys) );

        SDL_Event event;
        while( SDL_PollEvent(&event) ) {
            if( event.type == SDL_QUIT ) {
                isRunning = false;
            } else if( event.type == SDL_WINDOWEVENT  && (
                   event.window.event == SDL_WINDOWEVENT_RESIZED ||
                   event.window.event == SDL_WINDOWEVENT_EXPOSED) ) {
                SDL_GetWindowSize( window, &windowWidth, &windowHeight );
            } else if( event.type == SDL_KEYDOWN ) {
                // NOTE(JRC): The keys processed here are those that are pressed
                // but not held. This type of processing is good for one-time
                // events (e.g. recording, replaying, etc.).
                SDL_Keycode pressedKey = event.key.keysym.sym;
                if( pressedKey == SDLK_q ) {
                    isRunning = false;
                }
#ifdef LLCE_DEBUG
                else if( pressedKey == SDLK_t && !isRecording ) {
                    if( !isReplaying ) {
                        repFrameIdx = 0;
                        recStateStream.open( cStateFilePath, cIOModeR );
                        recInputStream.open( cInputFilePath, cIOModeR );
                        recInputStream.seekg( 0, std::ios_base::end );
                    } else {
                        repFrameIdx = 0;
                        recStateStream.close();
                        recInputStream.close();
                    }
                    isReplaying = !isReplaying;
                } else if( pressedKey == SDLK_r && !isReplaying ) {
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
#endif
            }
        }

#ifdef LLCE_DEBUG
        // TODO(JRC): This is a bit weird for replaying because we allow intercepts
        // from any key before replacing all key presses with replay data. This is
        // good in some ways as it allows recordings to be excited, but it does
        // open the door for weird behavior like embedded recordings.
        if( isRecording ) {
            recInputStream.write( (bit8_t*)input->keys, sizeof(input->keys) );
            recFrameCount++;
        } if( isReplaying ) {
            if( recInputStream.peek() == EOF || recInputStream.eof() ) {
                repFrameIdx = 0;
                recStateStream.seekg( 0 );
                recStateStream.read( mem.buffer(), mem.length() );
                recInputStream.seekg( 0 );
            }
            recInputStream.read( (bit8_t*)input->keys, sizeof(input->keys) );
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

            llce::platform::dllUnloadHandle( hmpLibHandle, cDLLFileName );
            hmpLibHandle = llce::platform::dllLoadHandle( cDLLFileName );
            updateFunction = (update_f)llce::platform::dllLoadSymbol( hmpLibHandle, "update" );
            renderFunction = (render_f)llce::platform::dllLoadSymbol( hmpLibHandle, "render" );
            LLCE_ASSERT_ERROR(
                hmpLibHandle != nullptr && updateFunction != nullptr && renderFunction != nullptr,
                "Couldn't load library `" << cDLLFileName << "` symbols at " <<
                "simulation time " << simTimer.tt() << "." );

            prevDylibModTime = currDylibModTime;
        }
#endif

        glViewport( 0, 0, windowWidth, windowHeight );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity();
        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();
        glOrtho( -1.0f, +1.0f, -1.0f, +1.0f, -1.0f, +1.0f );

        glPushMatrix(); {
            updateFunction( state, input );
            renderFunction( state, input );
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
        state->time += simTimer.ft();
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
