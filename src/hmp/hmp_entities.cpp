#include <cstring>
#include <sstream>

#include <glm/common.hpp>
#include <glm/geometric.hpp>
#include <glm/ext/vector_float2.hpp>
#include <SDL2/SDL_opengl.h>

#include "hmp_entities.h"

namespace hmp {

/// 'hmp::bounds_t' Functions ///

bounds_t::bounds_t( const box_t& pBBox ) :
        entity_t( pBBox, {0x00, 0x2b, 0x36, 0xFF} ) {
    
}

/// 'hmp::ball_t' Functions ///

ball_t::ball_t( const box_t& pBBox ) :
        entity_t( pBBox, {0x80, 0x7e, 0x76, 0xFF} ) {
    
}


void ball_t::ricochet( const entity_t* pSurface ) {
    // TODO(JRC): Implement the following pseudocode for this function:
    //   calculate x, y interval intersections
    //   calculate contact normal as distance from surface to ball
    //     with components activated by x, y interval intersections
    //   generate a new velocity vector as a weighted contribution
    //     of the original source vector and the velocity of the surface

    // mVel
}

/// 'hmp::paddle_t' Functions ///

paddle_t::paddle_t( const box_t& pBBox, const color_t& pColor ) :
        entity_t( pBBox, pColor ), mDX( 0 ), mDY( 0 ) {
    
}


void paddle_t::move( const int32_t pDX, const int32_t pDY ) {
    mDX = glm::clamp( pDX, -1, 1 );
    mDY = glm::clamp( pDY, -1, 1 );
}


void paddle_t::iupdate( const float64_t pDT ) {
    mVel.x = mDX * paddle_t::MOVE_VEL;
    mVel.y = mDY * paddle_t::MOVE_VEL;
    entity_t::iupdate( pDT );
}

}
