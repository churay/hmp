#ifndef HMP_BOX_T_H
#define HMP_BOX_T_H

#include <glm/ext/vector_float2.hpp>

#include "hmp_interval_t.h"

#include "consts.h"

namespace hmp {

class box_t {
    public:

    /// Constructors ///

    box_t( const glm::vec2& pPos, const glm::vec2& pDims );

    /// Class Functions ///

    bool32_t embed( const box_t& pOther );

    bool32_t contains( const glm::vec2& pPos ) const;
    bool32_t contains( const box_t& pOther ) const;
    bool32_t overlaps( const box_t& pOther ) const;

    bool32_t empty() const;
    bool32_t valid() const;

    interval_t xbounds() const;
    interval_t ybounds() const;

    /// Class Fields ///

    glm::vec2 mPos, mDims;
};

}

#endif
