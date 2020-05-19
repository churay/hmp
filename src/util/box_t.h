#ifndef LLCE_BOX_T_H
#define LLCE_BOX_T_H

#include <sstream>

#include "interval_t.h"
#include "geom.h"
#include "consts.h"

namespace llce {

class box_t {
    public:

    /// Constructors ///

    box_t();
    box_t(
        const vec2f32_t& pPos, const vec2f32_t& pDims,
        const llce::geom::anchor2D_e pAnchor = llce::geom::anchor2D::ll );
    box_t(
        const float32_t pPos, const float32_t pDims,
        const llce::geom::anchor2D_e pAnchor = llce::geom::anchor2D::ll );
    box_t(
        const float32_t pPosX, const float32_t pPosY,
        const float32_t pDimsX, const float32_t pDimsY,
        const llce::geom::anchor2D_e pAnchor = llce::geom::anchor2D::ll );

    /// Class Functions ///

    bool32_t embed( const box_t& pOther );
    bool32_t exbed(const box_t& pOther );

    bool32_t contains( const vec2f32_t& pPos ) const;
    bool32_t contains( const box_t& pOther ) const;
    bool32_t overlaps( const box_t& pOther ) const;
    box_t intersect( const box_t& pOther ) const;
    box_t unionize( const box_t& pOther ) const;

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
