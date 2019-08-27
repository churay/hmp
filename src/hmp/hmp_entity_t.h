#ifndef HMP_ENTITY_T_H
#define HMP_ENTITY_T_H

#include "hmp_box_t.h"

#include "hmp_consts.h"
#include "consts.h"

namespace hmp {

class entity_t {
    public:

    /// Constructors ///

    entity_t( const box_t& pBBox, const color4u8_t* pColor );

    /// Class Functions ///

    void update( const float64_t pDT );
    void render() const;

    /// Class Fields ///

    public:

    box_t mBBox; // units: world
    vec2f32_t mVel; // units: world / second
    const color4u8_t* mColor; // units: (r,g,b,a)
    float64_t mLifetime; // units: seconds
};

}

#endif
