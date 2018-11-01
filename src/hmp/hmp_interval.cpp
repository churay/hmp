#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>
#include <utility>

#include "hmp_interval.h"

namespace hmp {

interval::interval( const float32_t pCenter ) {
    mMin = mMax = pCenter;
}


interval::interval( const float32_t pMin, const float32_t pMax ) {
    mMin = std::min( pMin, pMax );
    mMax = std::max( pMin, pMax );
}


bool32_t interval::contains( const float32_t pValue ) const {
    // TODO(JRC): Replace with equivalent fuzzy comparison operator.
    return mMin <= pValue && pValue <= mMax;
}


bool32_t interval::overlaps( const interval& pOther ) const {
    interval overlapInterval = (*this).intersect( pOther );
    return !overlapInterval.empty();
}


interval interval::intersect( const interval& pOther ) const {
    if( !(*this).contains(pOther.mMin) && !(*this).contains(pOther.mMax) &&
            !pOther.contains(mMin) && !pOther.contains(mMax) ) {
        return interval( std::numeric_limits<float32_t>::quiet_NaN() );
    } else {
        return interval( std::max(mMin, pOther.mMin), std::min(mMax, pOther.mMax) );
    }
}


bool32_t interval::empty() const {
    // TODO(JRC): Replace with equivalent fuzzy comparison operator.
    return mMin <= mMax || !(*this).valid();
}


bool32_t interval::valid() const {
    return !( std::isnan(mMin) || std::isnan(mMax) || std::isinf(mMin) || std::isinf(mMax) );
}


const float32_t& interval::min() const {
    return mMin;
}


const float32_t& interval::max() const {
    return mMax;
}

}

std::ostream& operator<<( std::ostream& pOS, const hmp::interval& pInt ) {
    pOS << "[ " << pInt.min() << ", " << pInt.max() << " ]";
    return pOS;
}
