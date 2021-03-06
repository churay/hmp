#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_opengl_glext.h>
#include <SDL2/SDL_ttf.h>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <cmath>
#include <cstring>
#include <cstdio>
#include <fstream>

#include "timer_t.h"
#include "memory_t.h"
#include "buffer_t.h"
#include "path_t.h"
#include "platform.h"
#include "input.h"
#include "output.h"
#include "cli.h"
#include "box_t.h"
#include "util.hpp"
#include "consts.h"

#include LLCE_SIMULATION_HEADER
#ifdef LLCE_DEBUG
#include "meta/meta.h"
#endif

namespace llsim = LLCE_SIMULATION;

typedef std::ios_base::openmode ioflag_t;
typedef llce::platform::path_t path_t;
typedef llce::buffer_t buffer_t;

typedef const int64_t& (*reduce_f)( const int64_t&, const int64_t& );

typedef bool32_t (*init_f)( llsim::state_t*, llsim::input_t* );
typedef bool32_t (*boot_f)( llsim::output_t* );
typedef bool32_t (*update_f)( llsim::state_t*, llsim::input_t*, const llsim::output_t*, const float64_t );
typedef bool32_t (*render_f)( const llsim::state_t*, const llsim::input_t*, const llsim::output_t* );

int32_t main( const int32_t pArgCount, const char8_t* pArgs[] ) {
    /// Initialize Global Constant State ///

    const static float64_t csSimFPS = static_cast<float64_t>( LLCE_FPS );
    const static uint64_t csBackupBufferCount = LLCE_DEBUG ? 2 * LLCE_FPS : 0;

    /// Parse Input Arguments ///

    // -v: display version information
    if( llce::cli::exists("-v", pArgs, pArgCount) ) {
        LLCE_INFO_RELEASE( "{Version: v" << LLCE_VERSION << ", " <<
            "Build: " << (LLCE_DEBUG ? "Debug" : "Release") << ", " <<
            "Libraries: " << (LLCE_DYLOAD ? "Dynamic" : "Static") << ", " <<
            "Floats: " << (LLCE_FDOUBLE ? "Double" : "Single") << "-Precision, " <<
            "Capture*:" << (LLCE_CAPTURE ? "Enabled" : "Disabled") << "}" );
    }

    // -m: display a second window w/ meta information
    const bool32_t cShowMeta = LLCE_DEBUG ? llce::cli::exists( "-m", pArgs, pArgCount ) : false;
    const float32_t cShowMetaF = static_cast<float32_t>( cShowMeta );

    // -r [replay-id]: replay in simulation state
    const char8_t* cSimStateArg = llce::cli::value( "-r", pArgs, pArgCount );
    const int32_t cSimStateIdx = cSimStateArg != nullptr ? std::atoi( cSimStateArg ) : -1;
    const bool32_t cIsSimulating = LLCE_DEBUG ? cSimStateIdx > 0 : false;

    /// Initialize Application Memory/State ///

    // NOTE(JRC): This base address was chosen by following the steps enumerated
    // in the 'doc/static_address.md' documentation file.
    bit8_t* const cSimBufferAddress = LLCE_DEBUG ? (bit8_t*)0x0000100000000000 : nullptr;
    const uint64_t cSimBufferLength = llce::util::bytes<'M'>( 1 );
    const uint64_t cSimDataLength = sizeof( llsim::state_t ) + sizeof( llsim::input_t ) +
        sizeof( llsim::output_t ) + 4 * sizeof( size_t );

    llce::memory_t simMemory( cSimBufferLength, cSimDataLength, cSimBufferAddress );
    llsim::state_t* simState = (llsim::state_t*)simMemory.dalloc( sizeof(llsim::state_t) );
    llsim::input_t* simInput = (llsim::input_t*)simMemory.dalloc( sizeof(llsim::input_t) );
    llsim::output_t* simOutput = (llsim::output_t*)simMemory.dalloc( sizeof(llsim::output_t) );

#if LLCE_DEBUG
    // NOTE(JRC): This workaround for 'allocating' the 'backupStates' and 'backupInputs'
    // arrays is necessary because C++ default initialize the members of class/struct arrays
    // and the 'state_t'/'input_t' types do not support such initialization options.
    static bit8_t sBackupStateBuffer[csBackupBufferCount * sizeof(llsim::state_t)];
    llsim::state_t* backupStates = (llsim::state_t*)&sBackupStateBuffer[0];
    static bit8_t sBackupInputBuffer[csBackupBufferCount * sizeof(llsim::input_t)];
    llsim::state_t* backupInputs = (llsim::state_t*)&sBackupInputBuffer[0];
#endif

    llsim::input_t baseInput;

#if LLCE_DEBUG
    // NOTE(JRC): The 'meta' module doesn't participate in loop-live editing, so its
    // initialized with local data and the application's input.
    meta::state_t metaStateData;
    meta::output_t metaOutputData;
    meta::state_t* metaState = &metaStateData;
    meta::input_t* metaInput = (meta::input_t*)&baseInput;
    meta::output_t* metaOutput = &metaOutputData;
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

    const static char8_t* csDLLFileNames[] = { LLCE_SIMULATION_DATA_LIBRARY, LLCE_SIMULATION_SOURCE_LIBRARY };
    const static uint32_t csDLLCount = LLCE_ELEM_COUNT( csDLLFileNames );
    const uint32_t cDataDLLID = 0, cCodeDLLID = 1;

    path_t dllFilePaths[csDLLCount];
    void* dllHandles[csDLLCount];
#if LLCE_DYLOAD
    for( uint32_t dllIdx = 0; dllIdx < csDLLCount; dllIdx++ ) {
        const char8_t* cDLLFileName = csDLLFileNames[dllIdx];
        path_t& dllFilePath = dllFilePaths[dllIdx];
        void*& dllHandle = dllHandles[dllIdx];

        dllFilePath = llce::platform::libFindDLLPath( cDLLFileName );
        LLCE_ASSERT_ERROR( dllFilePath.exists(),
            "Failed to find library '" << cDLLFileName << "' in dynamic path." );
        dllHandle = nullptr;
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

    const vec2i32_t cWindowInitDims = { 640, 480 + cShowMetaF * (640 - 480) };
    // NOTE(JRC): This could become non-const if the window positions ever
    // become configurable.
    const llce::box_t cViewportBoxs[] = {
        llce::box_t(cShowMetaF * vec2f32_t(0.0f, 0.25f), vec2f32_t(1.0f, 1.0f) - cShowMetaF * vec2f32_t(0.0f, 0.25f)),
        llce::box_t(vec2f32_t(0.0f, 0.0f), cShowMetaF * vec2f32_t(1.0f, 0.25f)) };

    vec2i32_t windowDims = cWindowInitDims;
    vec2i32_t viewportRess[] = { {0, 0}, {0, 0} };
    vec2i32_t viewportPoss[] = { {0, 0}, {0, 0} };
    const uint32_t cSimViewportID = 0, cMetaViewportID = 1;
    const uint32_t cViewportCount = LLCE_ELEM_COUNT( viewportRess );

    const auto cRecalcViewports = [ &cViewportBoxs, &windowDims, &viewportRess, &viewportPoss ] () {
        const uint32_t cViewportCount = LLCE_ELEM_COUNT( viewportRess );
        for( uint32_t viewportIdx = 0; viewportIdx < cViewportCount; viewportIdx++ ) {
            viewportRess[viewportIdx] = {
                cViewportBoxs[viewportIdx].mDims.x * windowDims.x,
                cViewportBoxs[viewportIdx].mDims.y * windowDims.y
            };
            viewportPoss[viewportIdx] = {
                cViewportBoxs[viewportIdx].mPos.x * windowDims.x,
                cViewportBoxs[viewportIdx].mPos.y * windowDims.y
            };
        }
    };

    const auto cResetViewport = [ &viewportRess, &viewportPoss ] ( const uint32_t pViewportIdx ) {
        glViewport(
            viewportPoss[pViewportIdx].x, viewportPoss[pViewportIdx].y,
            viewportRess[pViewportIdx].x, viewportRess[pViewportIdx].y );
        glScissor(
            viewportPoss[pViewportIdx].x, viewportPoss[pViewportIdx].y,
            viewportRess[pViewportIdx].x, viewportRess[pViewportIdx].y );

        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();
        glOrtho( -1.0f, +1.0f, -1.0f, +1.0f, -1.0f, +1.0f );

        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity();
    };

    SDL_Window* window = nullptr;
    SDL_GLContext windowGL = nullptr;

    const uint32_t cWindowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE |
        ( cIsSimulating ? SDL_WINDOW_HIDDEN : 0 );

    { // Initialize Window //
        window = SDL_CreateWindow(
            "Handmade Pong",                            // Window Title
            SDL_WINDOWPOS_UNDEFINED,                    // Window X Position
            SDL_WINDOWPOS_UNDEFINED,                    // Window Y Position
            windowDims.x,                               // Window Width
            windowDims.y,                               // Window Height
            cWindowFlags );                             // Window Flags
        LLCE_ASSERT_ERROR( window != nullptr,
            "SDL failed to create window instance; " << SDL_GetError() );

        cRecalcViewports();

        const path_t cIconPath( 2, cAssetPath.cstr(), "icon.png" );
        static color4u8_t sIconBuffer[LLCE_MAX_RESOLUTION * LLCE_MAX_RESOLUTION];

        uint32_t iconWidth = 0, iconHeight = 0;
        LLCE_VERIFY_WARNING(
            llce::platform::pngLoad(cIconPath, (bit8_t*)&sIconBuffer[0], iconWidth, iconHeight),
            "Failed to load icon at path '" << cIconPath << "'." );

        const uint32_t cRMask = (SDL_BYTEORDER == SDL_BIG_ENDIAN) ? 0xff000000 : 0x000000ff;
        const uint32_t cGMask = (SDL_BYTEORDER == SDL_BIG_ENDIAN) ? 0x00ff0000 : 0x0000ff00;
        const uint32_t cBMask = (SDL_BYTEORDER == SDL_BIG_ENDIAN) ? 0x0000ff00 : 0x00ff0000;
        const uint32_t cAMask = (SDL_BYTEORDER == SDL_BIG_ENDIAN) ? 0x000000ff : 0xff000000;

        SDL_Surface* windowIcon = nullptr;
        windowIcon = SDL_CreateRGBSurfaceFrom( (bit8_t*)&sIconBuffer[0], iconWidth, iconHeight,
            sizeof(color4u8_t) * 8, sizeof(color4u8_t) * iconWidth, cRMask, cGMask, cBMask, cAMask );
        LLCE_CHECK_WARNING( windowIcon != nullptr,
            "Failed to load the icon for the application." );

        SDL_SetWindowIcon( window, windowIcon );

        SDL_FreeSurface( windowIcon );
    }

    { // Initialize Window Graphics //
        LLCE_ASSERT_ERROR(
            (windowGL = SDL_GL_CreateContext(window)) != nullptr,
            "SDL failed to generate OpenGL context; " << SDL_GetError() );

        { // Load OpenGL Extensions //
            const static char8_t* csGLExtensionNames[] = { "GL_EXT_framebuffer_object", "GL_EXT_framebuffer_blit" };
            const static uint32_t csGLExtensionCount = LLCE_ELEM_COUNT( csGLExtensionNames );
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

            glEnable( GL_SCISSOR_TEST );
            glEnable( GL_LINE_STIPPLE );
            glEnable( GL_LINE_SMOOTH );

            glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
            glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
        }
    }

    /// Initialize Audio ///

    const static uint32_t csSimAudioEnabled = llsim::output_t::NUM_SFX_BUFFERS > 0;

    const static uint32_t csAudioFrequency = LLCE_SPS;                                  // audio samples / second
    const static SDL_AudioFormat csAudioFormat = AUDIO_S16LSB;                          // audio sample data format
    const static uint32_t csAudioChannelCount = LLCE_MAX_CHANNELS;                      // audio channels (2: stereo)
    const static uint32_t csAudioSampleBytes = sizeof( int16_t ) * csAudioChannelCount; // audio bytes / sample

    const static uint32_t csAudioSamplesPerFrames = csAudioFrequency / csSimFPS;        // audio buffer size in audio frames
    const static uint32_t csAudioBytesPerFrame = csAudioSamplesPerFrames * csAudioSampleBytes; // per-frame audio buffer size in bytes
    const static uint32_t csAudioBufferFrames = 2;                                      // max number of frames in audio buffer
    int16_t audioBuffer[csAudioBufferFrames * csAudioSamplesPerFrames * csAudioChannelCount];

    SDL_AudioSpec tempAudioConfig = {0}; {
        tempAudioConfig.freq = csAudioFrequency;
        tempAudioConfig.format = csAudioFormat;
        tempAudioConfig.channels = csAudioChannelCount;
        tempAudioConfig.samples = csAudioSamplesPerFrames;
        tempAudioConfig.callback = nullptr;
    }
    const SDL_AudioSpec cWantAudioConfig = tempAudioConfig;
    SDL_AudioSpec realAudioConfig;

    SDL_AudioDeviceID audioDeviceID = 1;
    LLCE_ASSERT_ERROR(
        (audioDeviceID = SDL_OpenAudioDevice(
            nullptr, 0, &tempAudioConfig, &realAudioConfig, SDL_AUDIO_ALLOW_ANY_CHANGE)) >= 0,
        "SDL failed to initialize audio device; " << SDL_GetError() );
    LLCE_ASSERT_ERROR(
        cWantAudioConfig.channels == realAudioConfig.channels && cWantAudioConfig.format == realAudioConfig.format,
        "SDL failed to initialize audio device with correct format." );

    const auto cResetAudio = [ &audioDeviceID, &audioBuffer, &simOutput ] () {
        SDL_ClearQueuedAudio( audioDeviceID );
        simOutput->sfxBufferFrames[llce::output::BUFFER_SHARED_ID] = 0;
        std::memset( &audioBuffer[0], 0, sizeof(audioBuffer) );
    };

    SDL_PauseAudioDevice( audioDeviceID, !csSimAudioEnabled );

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
    const static color4u8_t csWhiteColor = { 0xff, 0xff, 0xff, 0xff };
    const static color4u8_t csRedColor = { 0xff, 0x00, 0x00, 0xff };
    const static color4u8_t csGreenColor = { 0x00, 0xff, 0x00, 0xff };
    const static color4u8_t csBlueColor = { 0x00, 0x00, 0xff, 0xff };

#if LLCE_DEBUG
    const static uint32_t csTextureTextLength = 20;
    uint32_t textureGLIDs[] = { 0, 0, 0, 0, 0 };
    color4u8_t textureColors[] = { {0xff, 0x00, 0x00, 0xff}, {0x00, 0xff, 0x00, 0xff}, {0x00, 0x00, 0xff, 0xff} };
    char8_t textureTexts[][csTextureTextLength] = { "FPS: ???", "Recording ???", "Replaying ???", "Time: ???", "Speed: ???x" };
    const uint32_t cFPSTextureID = 0, cRecTextureID = 1, cRepTextureID = 2, cTimeTextureID = 3, cSpeedTextureID = 4;

    const uint32_t cTextureCount = LLCE_ELEM_COUNT( textureGLIDs );
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

    const static uint32_t csCaptureBufferSize = LLCE_CAPTURE ? LLCE_MAX_RESOLUTION * LLCE_MAX_RESOLUTION : 1;
    static color4u8_t sCaptureBuffer[csCaptureBufferSize];

    /// Input Wrangling ///

    // NOTE(JRC): This is the true input stream for the application.
    // The simulation stream is kept separate so its inputs don't interfere
    // with application-level functionality (e.g. debugging contexts, etc.).
    llsim::input_t* appInput = &baseInput;

    const auto cIsKeyDown = [] ( const llsim::input_t* pInput, const SDL_Scancode pKeyCode ) {
        return pInput->isDownRaw( llce::input::stream_t(llce::input::device::keyboard, pKeyCode) );
    };
    const auto cIsKeyPressed = [] ( const llsim::input_t* pInput, const SDL_Scancode pKeyCode ) {
        return pInput->isPressedRaw( llce::input::stream_t(llce::input::device::keyboard, pKeyCode) );
    };
    const auto cIsKeyReleased = [] ( const llsim::input_t* pInput, const SDL_Scancode pKeyCode ) {
        return pInput->isReleasedRaw( llce::input::stream_t(llce::input::device::keyboard, pKeyCode) );
    };

    const uint32_t cFXStreams[] = {
        llce::input::stream_t( llce::input::device::keyboard, SDL_SCANCODE_F1 ),
        llce::input::stream_t( llce::input::device::keyboard, SDL_SCANCODE_F2 ),
        llce::input::stream_t( llce::input::device::keyboard, SDL_SCANCODE_F3 ),
        llce::input::stream_t( llce::input::device::keyboard, SDL_SCANCODE_F4 ),
        llce::input::stream_t( llce::input::device::keyboard, SDL_SCANCODE_F5 ),
        llce::input::stream_t( llce::input::device::keyboard, SDL_SCANCODE_F6 ),
        llce::input::stream_t( llce::input::device::keyboard, SDL_SCANCODE_F7 ),
        llce::input::stream_t( llce::input::device::keyboard, SDL_SCANCODE_F8 ),
        llce::input::stream_t( llce::input::device::keyboard, SDL_SCANCODE_F9 ),
        llce::input::stream_t( llce::input::device::keyboard, SDL_SCANCODE_F10 ),
        llce::input::stream_t( llce::input::device::keyboard, SDL_SCANCODE_F11 ),
        llce::input::stream_t( llce::input::device::keyboard, SDL_SCANCODE_F12 )
    };

    /// Update/Render Loop ///

    bool32_t isRunning = true, isStepping = false, doStep = !isStepping;

    bool32_t isRecording = false, isReplaying = false;
    uint32_t currSlotIdx = 0, recSlotIdx = 0;
    uint32_t repFrameIdx = 0, recFrameCount = 0;

    int32_t simSpeedFactor = 0;

    bool32_t isCapturing = LLCE_CAPTURE & cIsSimulating;
    uint32_t currCaptureIdx = 0;

    llce::timer_t simTimer( csSimFPS, llce::timer_t::ratio_e::fps );
    float64_t simDT = 0.0, simWT = 0.0;
    // NOTE(JRC): A cursory check shows that it will take ~1e10 years of
    // uninterrupted run time for this to overflow at 60 FPS, so the fact
    // that this increments very quickly over time isn't a big concern.
    uint64_t simFrame = 0;

    isRunning &= dllInit( simState, simInput );
    isRunning &= dllBoot( simOutput );
#if LLCE_DEBUG
    if( cShowMeta ) {
        isRunning &= meta::init( metaState, metaInput );
        isRunning &= meta::boot( metaOutput );
    }
#endif

    if( csSimAudioEnabled ) {
        simOutput->sfxConfig = realAudioConfig;
        simOutput->sfxBuffers[llce::output::BUFFER_SHARED_ID] = (bit8_t*)&audioBuffer[0];
        simOutput->sfxBufferFrames[llce::output::BUFFER_SHARED_ID] = 0;
#if LLCE_DEBUG
        // if( cShowMeta ) {
        //     metaOutput->sfxConfig = realAudioConfig;
        //     metaOutput->sfxBuffers[llce::output::BUFFER_SHARED_ID] = (bit8_t*)&audioBuffer[0];
        //     metaOutput->sfxBufferFrames[llce::output::BUFFER_SHARED_ID] = 0;
        // }
#endif
    }

    while( isRunning ) {
        simTimer.split();

#if LLCE_DEBUG
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

        // TODO(JRC): This section takes a very long time to complete (O(1e-1)
        // seconds) when many inputs are pressed on the same frame (e.g. O(1e1)).
        // Disabling polling causes all inputs to be ignored (even by direct
        // input reading functions), so its unclear how to fix this issue.
        SDL_Event event;
        while( SDL_PollEvent(&event) ) {
            if( event.type == SDL_QUIT ) {
                isRunning = false;
            } else if( event.type == SDL_WINDOWEVENT && (
                   event.window.event == SDL_WINDOWEVENT_RESIZED ||
                   event.window.event == SDL_WINDOWEVENT_EXPOSED) ) {
                SDL_GetWindowSize( window, &windowDims.x, &windowDims.y );
                cRecalcViewports();
            }
        }

        appInput->read();

        if( cIsKeyPressed(appInput, SDL_SCANCODE_Q) ) {
            // q key = quit application
            isRunning = false;
        } if( cIsKeyPressed(appInput, SDL_SCANCODE_GRAVE) ) {
            // ` key = capture application
            isCapturing = true;
        }
#if LLCE_DEBUG
        if( cIsKeyPressed(appInput, SDL_SCANCODE_SPACE) ) {
            // space key = toggle frame advance mode
            LLCE_INFO_DEBUG( "Frame Advance <" << (!isStepping ? "ON " : "OFF") << ">" );
            isStepping = !isStepping;
            doStep = !isStepping;
        } if(cIsKeyPressed(appInput, SDL_SCANCODE_RETURN)) {
            // return key = advance during frame advance mode
            doStep = true;
        }

        if( cIsKeyPressed(appInput, SDL_SCANCODE_TAB) ) {
            // tab key = manipulate simulation speed
            bool32_t doSlowDown = cIsKeyDown( appInput, SDL_SCANCODE_LSHIFT );
            // TODO(JRC): Enable sped up playback once it can be reasonably achieved
            // (probably need to disable rendering frames in excess of 60 FPS).
            simSpeedFactor = glm::clamp( simSpeedFactor + (doSlowDown ? -1 : 1), -2, 0 );
            LLCE_INFO_DEBUG( "Playback Factor <" << simSpeedFactor << ">" );
        }

        if( (currSlotIdx = appInput->isPressedRaw(&cFXStreams[0])) || (cIsSimulating && !isReplaying) ) {
            // function key (fx) = debug state operation
            currSlotIdx = currSlotIdx - cFXStreams[0] + 1;
            recSlotIdx = !cIsSimulating ? currSlotIdx : cSimStateIdx;

            char8_t slotStateFileName[csOutputFileNameLength];
            char8_t slotInputFileName[csOutputFileNameLength];

            std::snprintf( &slotStateFileName[0], sizeof(slotStateFileName),
                cStateFileFormat, recSlotIdx );
            std::snprintf( &slotInputFileName[0], sizeof(slotInputFileName),
                cInputFileFormat, recSlotIdx );

            path_t slotStateFilePath( 2, cOutputPath.cstr(), &slotStateFileName[0] );
            path_t slotInputFilePath( 2, cOutputPath.cstr(), &slotInputFileName[0] );

            if( (cIsKeyDown(appInput, SDL_SCANCODE_LSHIFT) && !isRecording) || cIsSimulating ) {
                // lshift + fx = toggle slot x replay
                LLCE_INFO_DEBUG( "Replay Slot {" << recSlotIdx << "} <" << (!isReplaying ? "ON " : "OFF") << ">" );
                if( !isReplaying ) {
                    repFrameIdx = 0;
                    recStateStream.open( slotStateFilePath, cIOModeR );
                    recInputStream.open( slotInputFilePath, cIOModeR );

                    recFrameCount = (uint32_t)recInputStream.tellg();
                    recInputStream.seekg( 0, std::ios_base::end );
                    recFrameCount = (uint32_t)recInputStream.tellg() - recFrameCount;
                    recFrameCount /= sizeof( llsim::input_t );
                } else {
                    repFrameIdx = 0;
                    recStateStream.close();
                    recInputStream.close();
                }
                isReplaying = !isReplaying;
            } else if( cIsKeyDown(appInput, SDL_SCANCODE_RSHIFT) && !isRecording ) {
                // rshift + fx = hotload slot x state (reset replay)
                LLCE_INFO_DEBUG( "Hotload Slot {" << recSlotIdx << "}" );
                if( isReplaying ) {
                    repFrameIdx = 0;
                    recInputStream.seekg( 0, std::ios_base::end );
                } else {
                    recStateStream.open( slotStateFilePath, cIOModeR );
                    recInputStream.seekg( 0 );
                    recStateStream.read( (bit8_t*)simState, sizeof(llsim::state_t) );
                    recStateStream.close();
                }
            } else if( recSlotIdx != 1 && !isReplaying ) {
                // fx = toggle slot x recording
                LLCE_INFO_DEBUG( "Record Slot {" << recSlotIdx << "} <" << (!isRecording ? "ON " : "OFF") << ">" );
                if( !isRecording ) {
                    recFrameCount = 0;
                    recStateStream.open( slotStateFilePath, cIOModeW );
                    recStateStream.write( (bit8_t*)simState, sizeof(llsim::state_t) );
                    recStateStream.close();
                    recInputStream.open( slotInputFilePath, cIOModeW );
                } else {
                    recInputStream.close();
                }
                isRecording = !isRecording;
            } else if( recSlotIdx == 1 && !isReplaying ) {
                // f1 = instant backup record
                LLCE_INFO_DEBUG( "Hotsave Slot {" << recSlotIdx << "}" );
                // TODO(JRC): It's potentially worth putting a guard on this
                // function or improving the backup state implementation so
                // that hot-saving before the number of total backups is possible.
                uint64_t backupStartIdx = simFrame % csBackupBufferCount;
                recStateStream.open( slotStateFilePath, cIOModeW );
                recStateStream.write( (bit8_t*)&backupStates[backupStartIdx], sizeof(llsim::state_t) );
                recStateStream.close();

                recInputStream.open( slotInputFilePath, cIOModeW );
                for( uint32_t bufferIdx = 0; bufferIdx < csBackupBufferCount; bufferIdx++ ) {
                    uint64_t bbIdx = (backupStartIdx + bufferIdx) % csBackupBufferCount;
                    recInputStream.write( (bit8_t*)&backupInputs[bbIdx], sizeof(llsim::input_t) );
                }
                recInputStream.close();
            }
        }
#endif

        { // Initialize Graphics State //
            cResetViewport( cSimViewportID );
        }

        { // Initialize Audio State //
            std::memset( audioBuffer, 0, sizeof(audioBuffer) );

            if( csSimAudioEnabled ) {
                const uint32_t cQueuedAudioBytes = SDL_GetQueuedAudioSize( audioDeviceID );
                const uint32_t cQueuedAudioFrames = cQueuedAudioBytes / csAudioBytesPerFrame;
                simOutput->sfxBufferFrames[llce::output::BUFFER_SHARED_ID] = csAudioBufferFrames - cQueuedAudioFrames;
#if LLCE_DEBUG
                // metaOutput->sfxBufferFrames[llce::output::BUFFER_SHARED_ID] = simOutput->sfxBufferFrames[llce::output::BUFFER_SHARED_ID];
#endif
            }
        }

        if( doStep ) {
            simInput->read();
#if LLCE_DEBUG
            if( isRecording ) {
                recInputStream.write( (bit8_t*)simInput, sizeof(llsim::input_t) );
                recFrameCount++;
            } if( isReplaying ) {
                if( recInputStream.peek() == EOF || recInputStream.eof() ) {
                    isRunning = !( cIsSimulating && repFrameIdx != 0 );
                    repFrameIdx = 0;
                    recStateStream.seekg( 0 );
                    recStateStream.read( (bit8_t*)simState, sizeof(llsim::state_t) );
                    recInputStream.seekg( 0 );
                }
                recInputStream.read( (bit8_t*)simInput, sizeof(llsim::input_t) );
                repFrameIdx++;
            }
#endif

            isRunning &= dllUpdate( simState, simInput, simOutput, simDT );
            isRunning &= dllRender( simState, simInput, simOutput );

#if LLCE_DEBUG
            // TODO(JRC): It may be worth experimenting with allowing for the
            // saving of inputs during replaying/recording to allow for building
            // on old replays with new inputs.
            if( !isRecording && !isReplaying ) {
                uint64_t backupIdx = simFrame % csBackupBufferCount;
                std::memcpy( (void*)&backupStates[backupIdx], (void*)simState, sizeof(llsim::state_t) );
                std::memcpy( (void*)&backupInputs[backupIdx], (void*)simInput, sizeof(llsim::input_t) );
            }
#endif
        }

        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        glPushMatrix(); {
            // NOTE(JRC): This code calculates the normalized fit dimensions of
            // the simulation window (fixed aspect ratio) within the harness window
            // (variable aspect ratio based on user window manipulation).
            vec2f32_t simFitDims; {
                const vec2f32_t& cWindowDims = viewportRess[cSimViewportID];
                const vec2f32_t& cSimDims = simOutput->gfxBufferRess[llce::output::BUFFER_SHARED_ID];
                const float32_t cSimToWindowFactor =
                    ( llce::gfx::aspect(cWindowDims) < llce::gfx::aspect(cSimDims) ) ?
                    ( cWindowDims.x / (cSimDims.x + 0.0f) ) : ( cWindowDims.y / (cSimDims.y + 0.0f) );

                simFitDims = cSimToWindowFactor * cSimDims; // window space
                simFitDims = { simFitDims.x / cWindowDims.x, simFitDims.y / cWindowDims.y }; // norm space
            } const vec2f32_t cSimFitDims = simFitDims;

            mat4f32_t matWorldView( 1.0f );
            matWorldView *= glm::translate( vec3f32_t(-1.0f, -1.0f, 0.0f) );
            matWorldView *= glm::scale( vec3f32_t(2.0f, 2.0f, 1.0f) );
            matWorldView *= glm::translate( vec3f32_t((1.0f-cSimFitDims.x)/2.0f, (1.0f-cSimFitDims.y)/2.0f, 0.0f) );
            matWorldView *= glm::scale( vec3f32_t(cSimFitDims.x, cSimFitDims.y, 1.0f) );
            glMultMatrixf( &matWorldView[0][0] );

            glEnable( GL_TEXTURE_2D ); {
                // NOTE(JRC): This is required to get the expected/correct texture color,
                // but it's unclear as to why. OpenGL may perform color mixing by default?
                glColor4ubv( (uint8_t*)&csWhiteColor );
                glBindTexture( GL_TEXTURE_2D, simOutput->gfxBufferCBOs[llce::output::BUFFER_SHARED_ID] );
                glBegin( GL_QUADS ); {
                    glTexCoord2f( 0.0f, 0.0f ); glVertex2f( 0.0f, 0.0f );
                    glTexCoord2f( 0.0f, 1.0f ); glVertex2f( 0.0f, 1.0f );
                    glTexCoord2f( 1.0f, 1.0f ); glVertex2f( 1.0f, 1.0f );
                    glTexCoord2f( 1.0f, 0.0f ); glVertex2f( 1.0f, 0.0f );
                } glEnd();
                glBindTexture( GL_TEXTURE_2D, 0 );
            } glDisable( GL_TEXTURE_2D );
        } glPopMatrix();

        if( csSimAudioEnabled && simOutput->sfxBufferFrames[llce::output::BUFFER_SHARED_ID] > 0 && !cIsSimulating ) {
            SDL_QueueAudio( audioDeviceID, &audioBuffer[0],
                simOutput->sfxBufferFrames[llce::output::BUFFER_SHARED_ID] * csAudioBytesPerFrame );
            simOutput->sfxBufferFrames[llce::output::BUFFER_SHARED_ID] = 0;
        }

#if LLCE_DEBUG
        glEnable( GL_TEXTURE_2D ); {
            glColor4ubv( (uint8_t*)&csWhiteColor );

            std::snprintf( &textureTexts[cFPSTextureID][0],
                csTextureTextLength,
                "FPS: %0.2f", 1.0 / simDT );
            cGenerateTextTexture( cFPSTextureID, textureColors[cFPSTextureID], textureTexts[cFPSTextureID] );

            glBindTexture( GL_TEXTURE_2D, textureGLIDs[cFPSTextureID] );
            glBegin( GL_QUADS ); {
                glTexCoord2f( 0.0f, 0.0f ); glVertex2f( -1.0f + 0.0f, -1.0f + 0.2f ); // UL
                glTexCoord2f( 0.0f, 1.0f ); glVertex2f( -1.0f + 0.0f, -1.0f + 0.0f ); // BL
                glTexCoord2f( 1.0f, 1.0f ); glVertex2f( -1.0f + 0.5f, -1.0f + 0.0f ); // BR
                glTexCoord2f( 1.0f, 0.0f ); glVertex2f( -1.0f + 0.5f, -1.0f + 0.2f ); // UR
            } glEnd();

            std::snprintf( &textureTexts[cSpeedTextureID][0],
                csTextureTextLength,
                "Speed: %3.1fx", std::pow(2.0f, simSpeedFactor + 0.0f) );
            cGenerateTextTexture( cSpeedTextureID, textureColors[cFPSTextureID], textureTexts[cSpeedTextureID] );

            glBindTexture( GL_TEXTURE_2D, textureGLIDs[cSpeedTextureID] );
            glBegin( GL_QUADS ); {
                glTexCoord2f( 0.0f, 0.0f ); glVertex2f( +1.0f - 0.5f, -1.0f + 0.2f ); // UL
                glTexCoord2f( 0.0f, 1.0f ); glVertex2f( +1.0f - 0.5f, -1.0f + 0.0f ); // BL
                glTexCoord2f( 1.0f, 1.0f ); glVertex2f( +1.0f + 0.0f, -1.0f + 0.0f ); // BR
                glTexCoord2f( 1.0f, 0.0f ); glVertex2f( +1.0f + 0.0f, -1.0f + 0.2f ); // UR
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

#if LLCE_CAPTURE
        if( isCapturing ) {
            LLCE_INFO_RELEASE( "Capture Slot {" << recSlotIdx << "-" << currCaptureIdx << "}" );

            char8_t slotCaptureFileName[csOutputFileNameLength];
            std::snprintf( &slotCaptureFileName[0],
                sizeof(slotCaptureFileName),
                cRenderFileFormat, recSlotIdx, currCaptureIdx++ );
            path_t capturePath( 2, cOutputPath.cstr(), slotCaptureFileName );

            glEnable( GL_TEXTURE_2D );
            glBindTexture( GL_TEXTURE_2D, simOutput->gfxBufferCBOs[llce::output::BUFFER_SHARED_ID] );

            // TODO(JRC): Reversing the colors results in the proper color values,
            // but it's unclear why this is necessary given that they're stored
            // internally in the order requested. Debugging may be required in the
            // future when adapting this code to work on multiple platforms.
            bool8_t doWindowCapture = cIsKeyDown( appInput, SDL_SCANCODE_LSHIFT );
            vec2u32_t captureDims( 0, 0 );
            if( doWindowCapture ) {
                captureDims = windowDims;
                glReadPixels( 0, 0, windowDims.x, windowDims.y, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, sCaptureBuffer );
            } else { // if( doBufferCapture ) {
                captureDims = simOutput->gfxBufferRess[llce::output::BUFFER_SHARED_ID];
                glGetTexImage( GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, sCaptureBuffer );
            }

            // TODO(JRC): Ultimately, it would be best if the data could just
            // be funneled natively into the PNG interface instead of having
            // to mirror it about the y=h/2 axis.
            color4u8_t tempBuffer[LLCE_MAX_RESOLUTION];
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

            glBindTexture( GL_TEXTURE_2D, 0 );
            glDisable( GL_TEXTURE_2D );
        }
        isCapturing = cIsSimulating;
#endif

#if LLCE_DEBUG
        if( cShowMeta ) {
            cResetViewport( cMetaViewportID );

            isRunning &= meta::update( metaState, metaInput, metaOutput, simDT - std::min(0.0, simWT) );
            isRunning &= meta::render( metaState, metaInput, metaOutput );

            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            glPushMatrix(); {
                mat4f32_t matWorldView( 1.0f );
                matWorldView *= glm::translate( vec3f32_t(-1.0f, -1.0f, 0.0f) );
                matWorldView *= glm::scale( vec3f32_t(2.0f, 2.0f, 1.0f) );
                glMultMatrixf( &matWorldView[0][0] );

                glEnable( GL_TEXTURE_2D ); {
                    // NOTE(JRC): This is required to get the expected/correct texture color,
                    // but it's unclear as to why. OpenGL may perform color mixing by default?
                    glColor4ubv( (uint8_t*)&csWhiteColor );
                    glBindTexture( GL_TEXTURE_2D, metaOutput->gfxBufferCBOs[llce::output::BUFFER_SHARED_ID] );
                    glBegin( GL_QUADS ); {
                        glTexCoord2f( 0.0f, 0.0f ); glVertex2f( 0.0f, 0.0f );
                        glTexCoord2f( 0.0f, 1.0f ); glVertex2f( 0.0f, 1.0f );
                        glTexCoord2f( 1.0f, 1.0f ); glVertex2f( 1.0f, 1.0f );
                        glTexCoord2f( 1.0f, 0.0f ); glVertex2f( 1.0f, 0.0f );
                    } glEnd();
                    glBindTexture( GL_TEXTURE_2D, 0 );
                } glDisable( GL_TEXTURE_2D );
            } glPopMatrix();
        }
#endif

        SDL_GL_SwapWindow( window );

        const float32_t cFrameFPS = csSimFPS * std::pow( 2.0f, simSpeedFactor + 0.0f );
        simTimer.split();
        simWT = cIsSimulating ? 0.0 : simTimer.wait( cFrameFPS );
        simDT = simTimer.ft( llce::timer_t::time_e::ideal );
        simFrame++;

        LLCE_ASSERT_WARNING( simWT >= 0.0 || simFrame == 0,
            "Frame {" << simFrame << "} lagged; achieved " <<
            1.0 / (simDT - simWT) << " fps for ideal " << csSimFPS << " fps!" );

        doStep = !isStepping;
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

    SDL_GL_DeleteContext( windowGL );
    SDL_DestroyWindow( window );

    SDL_Quit();

    return 0;
}
