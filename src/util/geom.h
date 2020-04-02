#ifndef LLCE_GEOM_H
#define LLCE_GEOM_H

#include <glm/ext/vector_float2.hpp>

#include "consts.h"

namespace llce {

namespace geom {

namespace anchor1D { enum anchor1D_e { lo = 0b00, mid = 0b01, hi = 0b10 }; };
namespace anchor2D { enum anchor2D_e { ll = 0b0000, lm = 0b0100, lh = 0b1000, ml = 0b0001, mm = 0b0101, mh = 0b1001, hl = 0b0010, hm = 0b0110, hh = 0b1010 }; };

typedef anchor1D::anchor1D_e anchor1D_e;
typedef anchor2D::anchor2D_e anchor2D_e;

float32_t anchor( const float32_t pInterval, const anchor1D_e pAnchor );
vec2f32_t anchor( const vec2f32_t pInterval, const anchor2D_e pAnchor );

}

}

#endif
