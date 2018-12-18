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

constexpr static float32_t UI_HEIGHT = 1.0e-1f;

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

/// Inpuit Types/Variables ///

constexpr static uint8_t KEY_DIFF_NONE = 0;
constexpr static uint8_t KEY_DIFF_DOWN = 1;
constexpr static uint8_t KEY_DIFF_UP = 2;

struct input_t {
    uint8_t keys[SDL_Scancode::SDL_NUM_SCANCODES];
    uint8_t diffs[SDL_Scancode::SDL_NUM_SCANCODES];
};

}

#endif
