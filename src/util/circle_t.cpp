#include <cmath>
#include <sstream>

#include <glm/common.hpp>
#include <glm/geometric.hpp>
#include <glm/gtx/string_cast.hpp>

#include "circle_t.h"

namespace llce {

/// Class Functions ///

circle_t::circle_t() :
        mCenter( 0.0f, 0.0f ), mRadius( 0.0f ) {
    
}

circle_t::circle_t( const vec2f32_t& pCenter, const float32_t pRadius ) :
        mCenter( pCenter ), mRadius( pRadius ) {
    
}


circle_t::circle_t( const float32_t pCenterX, const float32_t pCenterY, const float32_t pRadius ) :
        circle_t( {pCenterX, pCenterY}, pRadius ) {
    
}


bool32_t circle_t::embed( const circle_t& pOther ) {
    bool32_t success = pOther.mRadius >= mRadius;

    if( success && !pOther.contains(*this) ) {
        vec2f32_t toOther = pOther.mCenter - mCenter;
        vec2f32_t toEmbed = glm::normalize( toOther ) *
            ( glm::length(toOther) - mRadius + pOther.mRadius );
        mCenter += toEmbed;
    }

    return success;
}


bool32_t circle_t::exbed( const circle_t& pOther ) {
    if( (*this).overlaps(pOther) ) {
        vec2f32_t toSelf = mCenter - pOther.mCenter;
        vec2f32_t toExbed = glm::normalize( toSelf ) *
            ( mRadius + pOther.mRadius - glm::length(toSelf) );
        mCenter += toExbed;
    }

    return true;
}


bool32_t circle_t::contains( const vec2f32_t& pPos ) const {
    return glm::distance( mCenter, pPos ) <= mRadius;
}


bool32_t circle_t::contains( const circle_t& pOther ) const {
    return glm::distance( mCenter, pOther.mCenter ) + pOther.mRadius <= mRadius;
}


bool32_t circle_t::overlaps( const circle_t& pOther ) const {
    return glm::distance( mCenter, pOther.mCenter ) <= ( mRadius + pOther.mRadius );
}


bool32_t circle_t::empty() const {
    return mRadius <= 0.0f;
}


bool32_t circle_t::valid() const {
    return mRadius >= 0.0f;
}

}

std::ostream& operator<<( std::ostream& pOS, const llce::circle_t& pCircle ) {
    pOS << "(c" << glm::to_string(pCircle.mCenter) << ", r" << pCircle.mRadius << ")";
    return pOS;
}
