#include <sstream>

#include <glm/common.hpp>

#include "box_t.h"

namespace llce {

/// Class Functions ///

box_t::box_t() :
        mPos( 0.0f, 0.0f ), mDims( 0.0f, 0.0f ) {
    
}

box_t::box_t( const vec2f32_t& pPos, const vec2f32_t& pDims, const llce::geom::anchor2D_e pAnchor ) :
        mPos( pPos ), mDims( pDims ) {
    mPos -= llce::geom::anchor( pDims, pAnchor );
}


box_t::box_t( const float32_t pPos, const float32_t pDims, const llce::geom::anchor2D_e pAnchor ) :
        box_t( vec2f32_t(pPos, pPos), vec2f32_t(pDims, pDims), pAnchor ) {
    
}


box_t::box_t( const float32_t pPosX, const float32_t pPosY,
            const float32_t pDimsX, const float32_t pDimsY,
            const llce::geom::anchor2D_e pAnchor ) :
        box_t( vec2f32_t(pPosX, pPosY), vec2f32_t(pDimsX, pDimsY), pAnchor ) {
    
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

    float32_t minDeltaX = tx.exbed( ox ), minDeltaY = ty.exbed( oy );
    // TODO(JRC): Replace with equivalent fuzzy comparison operator.
    if( minDeltaX < minDeltaY ) {
        mPos = vec2f32_t( tx.mMin, mPos[1] );
        mDims = vec2f32_t( tx.mMax - tx.mMin, mDims[1] );
    } else {
        mPos = vec2f32_t( mPos[0], ty.mMin );
        mDims = vec2f32_t( mDims[0], ty.mMax - ty.mMin );
    }

    return true;
}


bool32_t box_t::contains( const vec2f32_t& pPos ) const {
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
    return box_t( vec2f32_t(bx.mMin, by.mMin),
        vec2f32_t(bx.mMax - bx.mMin, by.mMax - by.mMin) );
}


bool32_t box_t::empty() const {
    const interval_t tx = xbounds(), ty = ybounds();
    return tx.empty() || ty.empty();
}


bool32_t box_t::valid() const {
    const interval_t tx = xbounds(), ty = ybounds();
    return tx.valid() && ty.valid();
}


vec2f32_t box_t::min() const {
    return mPos;
}


vec2f32_t box_t::max() const {
    return mPos + mDims;
}


vec2f32_t box_t::center() const {
    return mPos + 0.5f * mDims;
}


float32_t box_t::ratio() const {
    return mDims.x / mDims.y;
}


interval_t box_t::xbounds() const {
    return interval_t( mPos[0], mPos[0] + mDims[0] );
}


interval_t box_t::ybounds() const {
    return interval_t( mPos[1], mPos[1] + mDims[1] );
}

}

std::ostream& operator<<( std::ostream& pOS, const llce::box_t& pBox ) {
    pOS << "(x" << pBox.xbounds() << ", y" << pBox.ybounds() << ")";
    return pOS;
}
