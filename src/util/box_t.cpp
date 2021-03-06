#include <sstream>

#include <glm/glm.hpp>

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
    return box_t( bx.mMin, by.mMin, bx.length(), by.length() );
}


box_t box_t::unionize( const box_t& pOther ) const {
    const interval_t tx = xbounds(), ty = ybounds();
    const interval_t ox = pOther.xbounds(), oy = pOther.ybounds();

    interval_t bx = tx.unionize( ox ), by = ty.unionize( oy );
    return box_t( bx.mMin, by.mMin, bx.length(), by.length() );
}


vec2f32_t box_t::wrap( const vec2f32_t& pValue ) const {
    const interval_t tx = xbounds(), ty = ybounds();
    return vec2f32_t( tx.wrap(pValue.x), ty.wrap(pValue.y) );
}


vec2f32_t box_t::clamp( const vec2f32_t& pValue ) const {
    const interval_t tx = xbounds(), ty = ybounds();
    return vec2f32_t( tx.clamp(pValue.x), ty.clamp(pValue.y) );
}


vec2f32_t box_t::distance( const vec2f32_t& pValue ) const {
    const interval_t tx = xbounds(), ty = ybounds();
    return vec2f32_t( tx.distance(pValue.x), ty.distance(pValue.y) );
}


vec2f32_t box_t::interp( const vec2f32_t& pValue ) const {
    return mPos + vec2f32_t( mDims.x * pValue.x, mDims.y * pValue.y );
}


vec2f32_t box_t::min() const {
    return mPos;
}


vec2f32_t box_t::max() const {
    return mPos + mDims;
}


vec2f32_t box_t::mid() const {
    return mPos + 0.5f * mDims;
}


vec2f32_t box_t::at( const llce::geom::anchor2D_e pAnchor ) const {
    return mPos + vec2f32_t(
        llce::geom::anchor(mDims.x, static_cast<llce::geom::anchor1D_e>(pAnchor >> 0 & 0b11)),
        llce::geom::anchor(mDims.y, static_cast<llce::geom::anchor1D_e>(pAnchor >> 2 & 0b11))
    );
}


float32_t box_t::area() const {
    return mDims.x * mDims.y;
}


bool32_t box_t::empty() const {
    const interval_t tx = xbounds(), ty = ybounds();
    return tx.empty() || ty.empty();
}


bool32_t box_t::valid() const {
    const interval_t tx = xbounds(), ty = ybounds();
    return tx.valid() && ty.valid();
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
