#ifndef DEMO_LIB_H
#define DEMO_LIB_H

#include <SDL2/SDL.h>

#include "input.h"
#include "consts.h"

namespace demo {

/// State Types/Variables ///

struct state_t {
    // FPS State //
    color4f32_t hsvColor;
};

/// Input Types/Variables ///

struct input_t {
    llce::input::keyboard_t keyboard;
};

/// Output Types/Variables ///

constexpr static uint32_t GFX_BUFFER_MASTER = 0;
constexpr static uint32_t GFX_BUFFER_COUNT = 1;

constexpr static uint32_t SFX_BUFFER_MASTER = 0;
constexpr static uint32_t SFX_BUFFER_COUNT = 1;

struct output_t {
    // Graphics Output //
    uint32_t gfxBufferFBOs[GFX_BUFFER_COUNT];   // frame buffers
    uint32_t gfxBufferCBOs[GFX_BUFFER_COUNT];   // color buffers
    uint32_t gfxBufferDBOs[GFX_BUFFER_COUNT];   // depth buffers
    vec2u32_t gfxBufferRess[GFX_BUFFER_COUNT];  // buffer resolutions

    // Audio Output //
    SDL_AudioSpec sfxConfig;
    bit8_t* sfxBuffers[SFX_BUFFER_COUNT];
    uint32_t sfxBufferFrames[SFX_BUFFER_COUNT];
};

}

#if !LLCE_DYLOAD
extern "C" {
    bool32_t init( demo::state_t* pState, demo::input_t* pInput );
    bool32_t boot( demo::output_t* pOutput );
    bool32_t update( demo::state_t* pState, demo::input_t* pInput, const demo::output_t* pOutput, const float64_t pDT );
    bool32_t render( const demo::state_t* pState, const demo::input_t* pInput, const demo::output_t* pOutput );
};
#endif

#endif
