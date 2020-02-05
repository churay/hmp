#ifndef LLCE_INTERVAL_T_H
#define LLCE_INTERVAL_T_H

#include <sstream>

#include "consts.h"

namespace llce {

class interval_t {
    public:

    /// Constructors ///

    interval_t( const float32_t pCenter = 0.0f );
    interval_t( const float32_t pMin, const float32_t pMax );

    /// Operators ///

    bool32_t operator==( const interval_t& pOther ) const;

    /// Class Functions ///

    bool32_t embed( const interval_t& pOther );
    float32_t exbed( const interval_t& pOther );

    float32_t wrap( const float32_t pValue ) const;

    bool32_t contains( const float32_t pValue ) const;
    bool32_t contains( const interval_t& pOther ) const;
    bool32_t overlaps( const interval_t& pOther ) const;
    interval_t intersect( const interval_t& pOther ) const;

    float32_t length() const;
    bool32_t empty() const;
    bool32_t valid() const;

    /// Class Fields ///

    float32_t mMin, mMax;
};

}

std::ostream& operator<<( std::ostream& pOS, const llce::interval_t& pInt );

#endif
