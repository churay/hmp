#ifndef HMP_LIB_H
#define HMP_LIB_H

#include <SDL2/SDL.h>

#include "hmp_entities.h"
#include "consts.h"

namespace hmp {

const static uint32_t MAX_ENTITIES = 16;

struct state_t {
    float64_t dt, tt;
    entity_t* entities[hmp::MAX_ENTITIES];
    bounds_t boundsEnt;
    paddle_t playerEnt;
};


struct input_t {
    uint8_t keys[SDL_Scancode::SDL_NUM_SCANCODES];
};

}

#endif
