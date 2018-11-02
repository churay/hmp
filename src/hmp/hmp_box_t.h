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

    // TODO(JRC): This should really be abstracted elsewhere once the
    // code defines some notion of an actor.
    void update( const float32_t pDX, const float32_t pDY );
    void render( const uint8_t* pColor = nullptr ) const;

    bool32_t contains( const float32_t pX, const float32_t pY ) const;
    bool32_t contains( const box_t& pOther ) const;
    bool32_t overlaps( const box_t& pOther ) const;

    bool32_t empty() const;
    bool32_t valid() const;

    /// Class Fields ///

    interval_t mX, mY;
};

}

#endif
