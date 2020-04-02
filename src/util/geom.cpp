
#include "geom.h"

namespace llce {

namespace geom {

/// 'llce::geom' Anchor Functions ///

float32_t anchor( const float32_t pInterval, const anchor1D_e pAnchor ) {
    return pInterval * ( static_cast<int32_t>(pAnchor) + 0.0f ) / 2.0f;
}

vec2f32_t anchor( const vec2f32_t pInterval, const anchor2D_e pAnchor ) {
    return vec2f32_t(
        pInterval.x * ( static_cast<int32_t>(pAnchor >> 0 & 0b11) + 0.0f ) / 2.0f,
        pInterval.y * ( static_cast<int32_t>(pAnchor >> 2 & 0b11) + 0.0f ) / 2.0f
    );
}

};

};
