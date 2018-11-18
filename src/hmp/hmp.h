#ifndef HMP_LIB_H
#define HMP_LIB_H

#include <SDL2/SDL.h>

#include "hmp_entities.h"
#include "hmp_consts.h"
#include "consts.h"

namespace hmp {

constexpr static uint32_t MAX_ENTITIES = 16;
constexpr static float32_t ROUND_START_TIME = 1.0f;

constexpr static color_t BG_COLOR = { 0x00, 0x2b, 0x36, 0xFF };
constexpr static color_t UI_COLOR = { 0x00, 0x2b, 0x36, 0xFF };

struct state_t {
    float64_t dt, rt, tt;
    bool32_t roundStarted;

    entity_t* entities[hmp::MAX_ENTITIES];
    bounds_t boundsEnt;
    bounds_t ricochetEnts[2];
    ball_t ballEnt;
    paddle_t paddleEnts[2];
};


struct input_t {
    uint8_t keys[SDL_Scancode::SDL_NUM_SCANCODES];
};

}

#endif
