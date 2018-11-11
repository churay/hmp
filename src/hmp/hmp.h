#ifndef HMP_LIB_H
#define HMP_LIB_H

#include <SDL2/SDL.h>

#include "hmp_entities.h"
#include "consts.h"

namespace hmp {

struct state_t {
    bounds_t boundsEnt;
    paddle_t playerEnt;
    float64_t dt, tt;
};


struct input_t {
    uint8_t keys[SDL_Scancode::SDL_NUM_SCANCODES];
};

}

#endif
