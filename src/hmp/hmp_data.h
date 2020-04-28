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

const extern float32_t VOLUME;
const extern float32_t BLIP_TIME;

};

};

#endif
