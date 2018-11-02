#include "hmp_box_t.h"

namespace hmp {

/// Class Functions ///

box_t::box_t(
    const float32_t pX, const float32_t pY,
    const float32_t pW, const float32_t pH ) :
        mX( pX, pX + pW ), mY( pY, pY + pH ) {
}


bool32_t box_t::contains( const float32_t pX, const float32_t pY ) const {
    return mX.contains( pX ) && mY.contains( pY );
}


bool32_t box_t::overlaps( const box_t& pOther ) const {
    return mX.overlaps( pOther.mX ) && mY.overlaps( pOther.mY );
}

}
