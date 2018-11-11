#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>
#include <utility>

#include "hmp_interval_t.h"

namespace hmp {

interval_t::interval_t( const float32_t pCenter ) {
    mMin = mMax = pCenter;
}


interval_t::interval_t( const float32_t pMin, const float32_t pMax ) {
    mMin = std::min( pMin, pMax );
    mMax = std::max( pMin, pMax );
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


bool32_t interval_t::contains( const float32_t pValue ) const {
    // TODO(JRC): Replace with equivalent fuzzy comparison operator.
    return mMin <= pValue && pValue <= mMax;
}


bool32_t interval_t::contains( const interval_t& pOther ) const {
    // TODO(JRC): Replace with equivalent fuzzy comparison operator.
    interval_t overlapInterval = (*this).intersect( pOther );
    return overlapInterval.valid() && overlapInterval == pOther;
}


bool32_t interval_t::overlaps( const interval_t& pOther ) const {
    interval_t overlapInterval = (*this).intersect( pOther );
    return !overlapInterval.empty();
}


interval_t interval_t::intersect( const interval_t& pOther ) const {
    if( !(*this).contains(pOther.mMin) && !(*this).contains(pOther.mMax) &&
            !pOther.contains(mMin) && !pOther.contains(mMax) ) {
        return interval_t( std::numeric_limits<float32_t>::quiet_NaN() );
    } else {
        return interval_t( std::max(mMin, pOther.mMin), std::min(mMax, pOther.mMax) );
    }
}


bool32_t interval_t::empty() const {
    // TODO(JRC): Replace with equivalent fuzzy comparison operator.
    return mMin <= mMax || !(*this).valid();
}


bool32_t interval_t::valid() const {
    return !( std::isnan(mMin) || std::isnan(mMax) || std::isinf(mMin) || std::isinf(mMax) );
}

}

std::ostream& operator<<( std::ostream& pOS, const hmp::interval_t& pInt ) {
    pOS << "[" << pInt.mMin << ", " << pInt.mMax << "]";
    return pOS;
}
