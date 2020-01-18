#ifndef LLCE_CIRCLE_T_H
#define LLCE_CIRCLE_T_H

#include <sstream>

#include <glm/common.hpp>

#include "consts.h"

namespace llce {

class circle_t {
    public:

    /// Constructors ///

    circle_t();
    circle_t( const vec2f32_t& pCenter, const float32_t pRadius );
    circle_t( const float32_t pCenterX, const float32_t pCenterY, const float32_t pRadius );

    /// Class Functions ///

    bool32_t embed( const circle_t& pOther );
    bool32_t exbed(const circle_t& pOther );

    bool32_t contains( const vec2f32_t& pPos ) const;
    bool32_t contains( const circle_t& pOther ) const;
    bool32_t overlaps( const circle_t& pOther ) const;

    bool32_t empty() const;
    bool32_t valid() const;

    /// Class Fields ///

    vec2f32_t mCenter;
    float32_t mRadius;
};

}

std::ostream& operator<<( std::ostream& pOS, const llce::circle_t& pCircle );

#endif
