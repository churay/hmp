#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>
#include <utility>

#include "interval_t.h"

namespace llce {

interval_t::interval_t( const float32_t pCenter ) {
    mMin = mMax = pCenter;
}


interval_t::interval_t( const float32_t pValue1, const float32_t pValue2,
        const llce::geom::anchor1D_e pAnchor ) {
    if( pAnchor == llce::geom::anchor1D::lo || pAnchor == llce::geom::anchor1D::hi ) {
        mMin = std::min( pValue1, pValue2 );
        mMax = std::max( pValue1, pValue2 );
    } else if( pAnchor == llce::geom::anchor1D::mid ) {
        mMin = pValue1 - pValue2 / 2.0f;
        mMax = pValue1 + pValue2 / 2.0f;
    }
}


bool32_t interval_t::operator==( const interval_t& pOther ) const {
    // TODO(JRC): Replace with equivalent fuzzy comparison operator.
    return mMin == pOther.mMin && mMax == pOther.mMax;
}


bool32_t interval_t::embed( const interval_t& pOther ) {
    // TODO(JRC): Replace with equivalent fuzzy comparison operator.
    bool32_t success = ( mMax - mMin ) <= ( pOther.mMax - pOther.mMin );
    if( success ) {
        float32_t minDelta = std::max( mMin, pOther.mMin ) - mMin;
        float32_t maxDelta = std::min( mMax, pOther.mMax ) - mMax;
        mMin += minDelta + maxDelta;
        mMax += minDelta + maxDelta;
    }
    return success;
}


float32_t interval_t::exbed( const interval_t& pOther ) {
    float32_t intDelta = 0.0f;

    float32_t minDelta = pOther.mMax - mMin;
    float32_t maxDelta = mMax - pOther.mMin;
    // TODO(JRC): Replace with equivalent fuzzy comparison operator.
    if( minDelta > 0.0f && maxDelta > 0.0f ) {
        intDelta = ( minDelta < maxDelta ) ? minDelta : -maxDelta;
        mMin += intDelta;
        mMax += intDelta;
    }

    return std::abs( intDelta );
}


float32_t interval_t::wrap( const float32_t pValue ) const {
    float32_t wrapValue = std::fmod( pValue, mMax - mMin );
    return ( wrapValue < 0.0f ) ? mMax + wrapValue : mMin + wrapValue;
}


float32_t interval_t::interp( const float32_t pValue ) const {
    return pValue * ( mMax - mMin ) + mMin;
}


bool32_t interval_t::contains( const float32_t pValue ) const {
    // TODO(JRC): Replace with equivalent fuzzy comparison operator.
    return mMin <= pValue && pValue <= mMax;
}


bool32_t interval_t::contains( const interval_t& pOther ) const {
    // TODO(JRC): Replace with equivalent fuzzy comparison operator.
    interval_t overlapInterval = intersect( pOther );
    return overlapInterval.valid() && overlapInterval == pOther;
}


bool32_t interval_t::overlaps( const interval_t& pOther ) const {
    return !intersect( pOther ).empty();
}


interval_t interval_t::intersect( const interval_t& pOther ) const {
    if( !contains(pOther.mMin) && !contains(pOther.mMax) &&
            !pOther.contains(mMin) && !pOther.contains(mMax) ) {
        return interval_t( std::numeric_limits<float32_t>::quiet_NaN() );
    } else {
        return interval_t( std::max(mMin, pOther.mMin), std::min(mMax, pOther.mMax) );
    }
}


interval_t interval_t::unionize( const interval_t& pOther ) const {
    // TODO(JRC): Consider returning an invalid 'interval_t' if the two argument
    // intervals don't overlap or abut.
    return interval_t( std::min(mMin, pOther.mMin), std::max(mMax, pOther.mMax) );
}


float32_t interval_t::length() const {
    return valid() ? mMax - mMin : 0.0f;
}


bool32_t interval_t::empty() const {
    // TODO(JRC): Replace with equivalent fuzzy comparison operator.
    return mMin >= mMax || !valid();
}


bool32_t interval_t::valid() const {
    return !( std::isnan(mMin) || std::isnan(mMax) || std::isinf(mMin) || std::isinf(mMax) );
}

}

std::ostream& operator<<( std::ostream& pOS, const llce::interval_t& pInt ) {
    pOS << "[" << pInt.mMin << ", " << pInt.mMax << "]";
    return pOS;
}
