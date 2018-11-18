#include <sstream>

#include <glm/ext/vector_float2.hpp>
#include <SDL2/SDL_opengl.h>

#include "hmp_box_t.h"

namespace hmp {

/// Class Functions ///

box_t::box_t( const glm::vec2& pPos, const glm::vec2& pDims ) :
        mPos( pPos ), mDims( pDims ) {
}


bool32_t box_t::embed( const box_t& pOther ) {
    interval_t tx = xbounds(), ty = ybounds();
    const interval_t ox = pOther.xbounds(), oy = pOther.ybounds();

    bool32_t success = tx.embed( ox ) && ty.embed( oy );
    if( success ) {
        mPos[0] = tx.mMin;
        mPos[1] = ty.mMin;
    }
    return success;
}


bool32_t box_t::exbed( const box_t& pOther ) {
    interval_t tx = xbounds(), ty = ybounds();
    const interval_t ox = pOther.xbounds(), oy = pOther.ybounds();

    bool success = tx.exbed( ox ) && ty.exbed( oy );
    if( success ) {
        // TODO(JRC): Take direction that is more parallel (higher
        // normalized dot product) to the direction vector (center to center).
    }

    return false;
}


bool32_t box_t::contains( const glm::vec2& pPos ) const {
    const interval_t tx = xbounds(), ty = ybounds();
    return tx.contains( pPos[0] ) && ty.contains( pPos[1] );
}


bool32_t box_t::contains( const box_t& pOther ) const {
    const interval_t tx = xbounds(), ty = ybounds();
    const interval_t ox = pOther.xbounds(), oy = pOther.ybounds();
    return tx.contains( ox ) && ty.contains( oy );
}


bool32_t box_t::overlaps( const box_t& pOther ) const {
    const interval_t tx = xbounds(), ty = ybounds();
    const interval_t ox = pOther.xbounds(), oy = pOther.ybounds();
    return tx.overlaps( ox ) && ty.overlaps( oy );
}


box_t box_t::intersect( const box_t& pOther ) const {
    const interval_t tx = xbounds(), ty = ybounds();
    const interval_t ox = pOther.xbounds(), oy = pOther.ybounds();

    interval_t bx = tx.intersect( ox ), by = ty.intersect( oy );
    return box_t( glm::vec2(bx.mMin, by.mMin),
        glm::vec2(bx.mMax - bx.mMin, by.mMax - by.mMin) );
}


bool32_t box_t::empty() const {
    const interval_t tx = xbounds(), ty = ybounds();
    return tx.empty() || ty.empty();
}


bool32_t box_t::valid() const {
    const interval_t tx = xbounds(), ty = ybounds();
    return tx.valid() && ty.valid();
}


glm::vec2 box_t::center() const {
    return mPos + 0.5f * mDims;
}


interval_t box_t::xbounds() const {
    return interval_t( mPos[0], mPos[0] + mDims[0] );
}


interval_t box_t::ybounds() const {
    return interval_t( mPos[1], mPos[1] + mDims[1] );
}

}

std::ostream& operator<<( std::ostream& pOS, const hmp::box_t& pBox ) {
    pOS << "(x" << pBox.xbounds() << ", y" << pBox.ybounds() << ")";
    return pOS;
}
