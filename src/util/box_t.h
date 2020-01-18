#ifndef LLCE_BOX_T_H
#define LLCE_BOX_T_H

#include <sstream>

#include <glm/ext/vector_float2.hpp>

#include "interval_t.h"

#include "consts.h"

namespace llce {

class box_t {
    public:

    /// Class Attributes ///

    enum class anchor_e : uint8_t { sw=0, se=1, nw=2, ne=3, c=7 };

    /// Constructors ///

    box_t();
    box_t(
        const vec2f32_t& pPos, const vec2f32_t& pDims,
        const anchor_e pAnch = anchor_e::sw );
    box_t(
        const float32_t pPos, const float32_t pDims,
        const anchor_e pType = anchor_e::sw );
    box_t(
        const float32_t pPosX, const float32_t pPosY,
        const float32_t pDimsX, const float32_t pDimsY,
        const anchor_e pType = anchor_e::sw );

    /// Class Functions ///

    bool32_t embed( const box_t& pOther );
    bool32_t exbed(const box_t& pOther );

    bool32_t contains( const vec2f32_t& pPos ) const;
    bool32_t contains( const box_t& pOther ) const;
    bool32_t overlaps( const box_t& pOther ) const;
    box_t intersect( const box_t& pOther ) const;

    bool32_t empty() const;
    bool32_t valid() const;

    vec2f32_t min() const;
    vec2f32_t max() const;
    vec2f32_t center() const;
    float32_t ratio() const;
    interval_t xbounds() const;
    interval_t ybounds() const;

    /// Class Fields ///

    vec2f32_t mPos, mDims;
};

}

std::ostream& operator<<( std::ostream& pOS, const llce::box_t& pBox );

#endif
