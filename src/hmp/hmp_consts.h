#ifndef HMP_CONSTS_H
#define HMP_CONSTS_H

#include "util.h"

typedef llce::util::color_t color_t;
typedef llce::util::uicoord32_t uicoord32_t;

namespace hmp {

namespace team { enum team_e { west, east, neutral }; };

constexpr static uint8_t WINNING_SCORE = 3;

constexpr static color_t COLOR_VOID = { 0xFF, 0xFF, 0xFF, 0xFF };
constexpr static color_t COLOR_BACKGROUND = { 0x00, 0x2b, 0x36, 0xFF };
constexpr static color_t COLOR_INTERFACE = { 0x58, 0x58, 0x58, 0xFF };
constexpr static color_t COLOR_BORDER = { 0x26, 0x26, 0x26, 0xFF };
constexpr static color_t COLOR_TEAMS[] = {
    {0x9a, 0x86, 0x00, 0xFF},
    {0x00, 0x9d, 0xa3, 0xFF},
    {0x80, 0x7e, 0x76, 0xFF} };

};

#endif
