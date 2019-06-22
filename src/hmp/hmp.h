#ifndef HMP_LIB_H
#define HMP_LIB_H

#include <SDL2/SDL.h>

#include "hmp_entities.h"
#include "hmp_rng_t.h"
#include "hmp_consts.h"
#include "input.h"
#include "consts.h"

namespace hmp {

/// State Types/Variables ///

constexpr static uint32_t MAX_ENTITIES = 16;
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

    // Game State //
    float64_t rt; // round time
    bool32_t roundStarted; // round flag
    team::team_e roundServer; // round starter flag

    entity_t* entities[hmp::MAX_ENTITIES];
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

/// Graphics Types/Variables ///

constexpr static uint32_t GFX_BUFFER_MASTER = 0, GFX_BUFFER_SIM = 1, GFX_BUFFER_UI = 2;
constexpr static uint32_t GFX_BUFFER_COUNT = 3;

struct graphics_t {
    uint32_t bufferFBOs[GFX_BUFFER_COUNT];
    uint32_t bufferTIDs[GFX_BUFFER_COUNT];
    uint32_t bufferDIDs[GFX_BUFFER_COUNT];
    vec2u32_t bufferRess[GFX_BUFFER_COUNT];
    box_t bufferBoxs[GFX_BUFFER_COUNT];
};

}

#endif
