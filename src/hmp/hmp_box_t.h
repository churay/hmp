#ifndef HMP_BOX_T_H
#define HMP_BOX_T_H

#include "hmp_interval_t.h"

#include "consts.h"

namespace hmp {

class box_t {
    public:

    /// Constructors ///

    box_t( const float32_t pX = 0.0f, const float32_t pY = 0.0f,
        const float32_t pW = 1.0f, const float32_t pH = 1.0f );

    /// Class Functions ///

    bool32_t contains( const float32_t pX, const float32_t pY ) const;
    bool32_t overlaps( const box_t& pOther ) const;

    private:

    /// Class Fields ///

    interval_t mX, mY;
};

}

#endif