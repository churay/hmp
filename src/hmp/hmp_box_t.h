#ifndef HMP_BOX_T_H
#define HMP_BOX_T_H

#include <sstream>
#include <glm/ext/vector_float2.hpp>

#include "hmp_interval_t.h"

#include "consts.h"

namespace hmp {

class box_t {
    public:

    /// Class Attributes ///

    enum class pos_type : int8_t { sw=0, se=1, nw=2, ne=3, c=7 };

    /// Constructors ///

    box_t();
    box_t( const glm::vec2& pPos, const glm::vec2& pDims, const pos_type pType = pos_type::sw );
    box_t( const float32_t pPosX, const float32_t pPosY, const float32_t pDimsX, const float32_t pDimsY, const pos_type pType = pos_type::sw );

    /// Class Functions ///

    bool32_t embed( const box_t& pOther );
    bool32_t exbed(const box_t& pOther );

    bool32_t contains( const glm::vec2& pPos ) const;
    bool32_t contains( const box_t& pOther ) const;
    bool32_t overlaps( const box_t& pOther ) const;
    box_t intersect( const box_t& pOther ) const;

    bool32_t empty() const;
    bool32_t valid() const;

    glm::vec2 min() const;
    glm::vec2 max() const;
    glm::vec2 center() const;
    interval_t xbounds() const;
    interval_t ybounds() const;

    /// Class Fields ///

    glm::vec2 mPos, mDims;
};

}

std::ostream& operator<<( std::ostream& pOS, const hmp::box_t& pBox );

#endif
