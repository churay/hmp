#ifndef HMP_INTERVAL_T_H
#define HMP_INTERVAL_T_H

#include "consts.h"

namespace hmp {

class interval_t {
    public:

    /// Constructors ///

    interval_t( const float32_t pCenter = 0.0f );
    interval_t( const float32_t pMin, const float32_t pMax );

    /// Class Functions ///

    bool32_t contains( const float32_t pValue ) const;
    bool32_t overlaps( const interval_t& pOther ) const;
    interval_t intersect( const interval_t& pOther ) const;

    bool32_t empty() const;
    bool32_t valid() const;

    const float32_t& min() const;
    const float32_t& max() const;

    private:

    /// Class Fields ///

    float32_t mMin, mMax;
};

}

#endif
