#include "hmp_box.h"

namespace hmp {

/// Class Functions ///

box::box(
    const float32_t pX, const float32_t pY,
    const float32_t pW, const float32_t pH ) :
        mX( pX, pX + pW ), mY( pY, pY + pH ) {
}


bool32_t box::contains( const float32_t pX, const float32_t pY ) const {
    return false;
}


bool32_t box::contains( const box& pOther ) const {
    return false;
}


bool32_t box::overlaps( const box& pOther ) const {
    return false;
}

}
