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
    glm::vec2 surfVelVec = ( glm::length(pSurface->mVel) > 1.0e-6f ) ?
        glm::normalize( pSurface->mVel ) : glm::vec2( 0.0f, 0.0f );

    glm::vec2 contactVec = mBBox.center() - pSurface->mBBox.center();
    glm::vec2 contactNormal = contactVec; {
        // TODO(JRC): If it becomes relevant, fix the case where the ball
        // is completely contained in the surface; these cases will need
        // some form of unique logic to handle.
        bool32_t surfContainsX = pSurface->mBBox.xbounds().contains( mBBox.xbounds() );
        bool32_t surfContainsY = pSurface->mBBox.ybounds().contains( mBBox.ybounds() );
        contactNormal.x *= ( !surfContainsX + 0.0f );
        contactNormal.y *= ( !surfContainsY + 0.0f );
        contactNormal = glm::normalize( contactNormal );
    }

    glm::vec2 ricochetVelVec = 0.5f * surfVelVec +
        0.5f * glm::normalize( glm::reflect(mVel, contactNormal) );

    // perform local 'exbed' by pushing out in direction of contact normal
    // by intersection depth (projection of COM direction vector onto contact normal)
    // mBBox.exbed( pSurface->mBBox );
    // mBBox.mPos += glm::dot() / glm::dot() ;
    mVel = glm::length( mVel ) * glm::normalize( ricochetVelVec );
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
