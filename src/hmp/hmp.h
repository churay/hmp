#ifndef HMP_LIB_H
#define HMP_LIB_H

#include <SDL2/SDL.h>

#include "hmp_entities.h"
#include "hmp_rng_t.h"
#include "hmp_gfx.h"
#include "hmp_sfx.h"
#include "hmp_consts.h"
#include "input.h"
#include "consts.h"

namespace hmp {

/// State Types/Variables ///

constexpr static float32_t ROUND_START_TIME = 1.0f;

constexpr static char8_t MENU_ITEM_TEXT[][32] = { "START", "EXIT " };
constexpr static uint32_t MENU_ITEM_COUNT = ARRAY_LEN( MENU_ITEM_TEXT );

struct state_t {
    // Global State //
    float64_t dt; // frame time
    float64_t tt; // total time

    mode::mode_e mid; // game mode
    mode::mode_e pmid; // pending mode

    rng_t rng; // random number generator
    sfx::synth_t synth; // sound effect synthesizer

    // Game State //
    float64_t rt; // round time
    bool32_t roundStarted; // round flag
    bool32_t roundPaused; // round pause flag
    team::team_e roundServer; // round starter flag

    bounds_t boundsEnt;
    bounds_t ricochetEnts[2];
    scoreboard_t scoreEnt;
    ball_t ballEnt;
    paddle_t paddleEnts[2];

    // Menu State //
    uint8_t menuIdx;
};

/// Input Types/Variables ///

struct input_t {
    llce::input::keyboard_t keyboard;
};

/// Output Types/Variables ///

constexpr static uint32_t GFX_BUFFER_MASTER = 0, GFX_BUFFER_SIM = 1, GFX_BUFFER_UI = 2;
constexpr static uint32_t GFX_BUFFER_COUNT = 3;

constexpr static uint32_t SFX_BUFFER_MASTER = 0;
constexpr static uint32_t SFX_BUFFER_COUNT = 1;

struct output_t {
    // Graphics Output //
    uint32_t gfxBufferFBOs[GFX_BUFFER_COUNT];   // frame buffers
    uint32_t gfxBufferCBOs[GFX_BUFFER_COUNT];   // color buffers
    uint32_t gfxBufferDBOs[GFX_BUFFER_COUNT];   // depth buffers
    vec2u32_t gfxBufferRess[GFX_BUFFER_COUNT];  // buffer resolutions
    box_t gfxBufferBoxs[GFX_BUFFER_COUNT];      // buffer locations

    // Audio Output //
    SDL_AudioSpec sfxConfig;
    bit8_t* sfxBuffers[SFX_BUFFER_COUNT];
    bool32_t sfxDirtyBits[SFX_BUFFER_COUNT];
};

}

#endif
