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

namespace sfx {

constexpr static float64_t VOLUME = 1000.0;
constexpr static float64_t BLIP_TIME = 1.0e-1;

constexpr static float64_t MID_C_FREQ = 261.626;
constexpr static float64_t MID_A_FREQ = 220.000;
constexpr static float64_t MID_F_FREQ = 174.614;

};

};

#endif
