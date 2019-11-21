#include "hmp_data.h"

namespace hmp {

const uint32_t RNG_SEED = 12345678;

namespace color {

const color4u8_t VOID = { 0xFF, 0xFF, 0xFF, 0xFF };
const color4u8_t BACKGROUND = { 0x00, 0x2b, 0x36, 0xFF };
const color4u8_t BACKGROUND2 = static_cast<uint8_t>( 2 ) * hmp::color::BACKGROUND;
const color4u8_t INTERFACE = { 0x58, 0x58, 0x58, 0xFF };
const color4u8_t BORDER = { 0x26, 0x26, 0x26, 0xFF };
const color4u8_t TEAM[3] = {
    {0x9a, 0x86, 0x00, 0xFF},    // west
    {0x00, 0x9d, 0xa3, 0xFF},    // east
    {0x80, 0x7e, 0x76, 0xFF} };  // neut

};

};
