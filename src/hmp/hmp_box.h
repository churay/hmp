#ifndef HMP_BOX_H
#define HMP_BOX_H

#include "hmp_interval.h"

#include "consts.h"

namespace hmp {

class box {
    public:

    /// Constructors ///

    box( const float32_t pX = 0.0f, const float32_t pY = 0.0f,
        const float32_t pW = 1.0f, const float32_t pH = 1.0f );

    /// Class Functions ///

    bool32_t contains( const float32_t pX, const float32_t pY ) const;
    bool32_t contains( const box& pOther ) const;
    bool32_t overlaps( const box& pOther ) const;

    private:

    /// Class Fields ///

    interval mX, mY;
};

}

#endif
