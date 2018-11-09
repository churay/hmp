#ifndef HMP_LIB_H
#define HMP_LIB_H

#include <SDL2/SDL.h>

#include "hmp_paddle_t.h"
#include "hmp_box_t.h"
#include "consts.h"

namespace hmp {

struct state_t {
    box_t boundsBox;
    uint8_t boundsColor[4]; // r, g, b, a
    paddle_t playerEnt;
    float64_t dt, tt;
};


struct input_t {
    uint8_t keys[SDL_Scancode::SDL_NUM_SCANCODES];
};

}

#endif
