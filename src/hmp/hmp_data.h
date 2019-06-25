#ifndef HMP_DATA_H
#define HMP_DATA_H

#include "consts.h"

namespace hmp {

const extern uint32_t RNG_SEED;

namespace color {

const extern color4u8_t VOID;
const extern color4u8_t BACKGROUND;
const extern color4u8_t BACKGROUND2;
const extern color4u8_t INTERFACE;
const extern color4u8_t BORDER;
const extern color4u8_t TEAM[3];

};

namespace gfx {

constexpr static uint32_t DIGIT_WIDTH = 5, DIGIT_HEIGHT = 7;
constexpr static float64_t DIGIT_ASPECT = ( DIGIT_WIDTH + 0.0 ) / ( DIGIT_HEIGHT + 0.0 );
const extern bool8_t ASCII_DIGIT_MAP[128][DIGIT_HEIGHT][DIGIT_WIDTH];

};

};

#endif
