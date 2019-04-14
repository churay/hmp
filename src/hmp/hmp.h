#ifndef HMP_LIB_H
#define HMP_LIB_H

#include <SDL2/SDL.h>

#include "hmp_entities.h"
#include "hmp_consts.h"
#include "consts.h"

namespace hmp {

/// State Types/Variables ///

constexpr static uint32_t MAX_ENTITIES = 16;
constexpr static float32_t ROUND_START_TIME = 1.0f;

struct state_t {
    float64_t dt, rt, tt;
    bool32_t roundStarted;

    entity_t* entities[hmp::MAX_ENTITIES];
    bounds_t boundsEnt;
    bounds_t ricochetEnts[2];
    scoreboard_t scoreEnt;
    ball_t ballEnt;
    paddle_t paddleEnts[2];
};

/// Input Types/Variables ///

constexpr static uint8_t KEY_DIFF_NONE = 0;
constexpr static uint8_t KEY_DIFF_DOWN = 1;
constexpr static uint8_t KEY_DIFF_UP = 2;

struct input_t {
    uint8_t keys[SDL_Scancode::SDL_NUM_SCANCODES];
    uint8_t diffs[SDL_Scancode::SDL_NUM_SCANCODES];
};

/// Graphics Types/Variables ///

constexpr static uint32_t GFX_BUFFER_MASTER = 0, GFX_BUFFER_SIM = 1, GFX_BUFFER_UI = 2;
constexpr static uint32_t GFX_BUFFER_COUNT = 3;

constexpr static color_t COLOR_WEST = { 0x9a, 0x86, 0x00, 0xFF };
constexpr static color_t COLOR_EAST = { 0x00, 0x9d, 0xa3, 0xFF };

struct graphics_t {
    uint32_t bufferFBOs[GFX_BUFFER_COUNT];
    uint32_t bufferTIDs[GFX_BUFFER_COUNT];
    uint32_t bufferRess[GFX_BUFFER_COUNT][2];
    box_t bufferBoxs[GFX_BUFFER_COUNT];
};

}

#endif
