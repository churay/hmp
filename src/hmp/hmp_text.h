#ifndef HMP_TEXT_H
#define HMP_TEXT_H

#include "hmp_consts.h"

namespace hmp {

namespace text {

constexpr static uint32_t DIGIT_WIDTH = 5, DIGIT_HEIGHT = 7;
const extern bool8_t ASCII_DIGIT_MAP[128][DIGIT_HEIGHT][DIGIT_WIDTH];

void render( const char8_t* pText, const color32_t* pColor );

};

};

#endif
