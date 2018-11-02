#ifndef HMP_LIB_H
#define HMP_LIB_H

#include <SDL2/SDL.h>

#include "hmp_box_t.h"
#include "consts.h"

namespace hmp {

struct state_t {
    box_t playerBox;
    uint8_t playerColor[4]; // r, g, b, a
    box_t boundsBox;
    uint8_t boundsColor[4]; // r, g, b, a
    float64_t time;
};


struct input_t {
    uint8_t keys[SDL_Scancode::SDL_NUM_SCANCODES];
};

}

#endif
