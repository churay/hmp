#include <cstring>
#include <sstream>

#include <glm/common.hpp>
#include <glm/ext/vector_float2.hpp>
#include <SDL2/SDL_opengl.h>

#include "hmp_entities.h"

namespace hmp {

/// 'hmp::bounds_t' Functions ///

bounds_t::bounds_t( const box_t& pBBox ) : entity_t( pBBox ) {
    const uint8_t dColor[4] = { 0x00, 0x2b, 0x36, 0xFF };
    std::memcpy( &mColor[0], &dColor, sizeof(dColor) );
}


void bounds_t::iupdate( const float64_t pDT ) {
    
}


void bounds_t::irender() const {
    glBegin( GL_QUADS ); {
        glColor4ubv( &mColor[0] );
        glVertex2f( 0.0f, 0.0f );
        glVertex2f( 1.0f, 0.0f );
        glVertex2f( 1.0f, 1.0f );
        glVertex2f( 0.0f, 1.0f );
    } glEnd();
};

/// 'hmp::paddle_t' Functions ///

paddle_t::paddle_t( const box_t& pBBox, const uint8_t* pColor ) :
        entity_t( pBBox ), mVel( 0.0f, 0.0f ), mDX( 0 ), mDY( 0 ) {
    std::memcpy( &mColor[0], pColor, sizeof(mColor) );
}


void paddle_t::move( const int32_t pDX, const int32_t pDY ) {
    mDX = glm::clamp( pDX, -1, 1 );
    mDY = glm::clamp( pDY, -1, 1 );
}


void paddle_t::iupdate( const float64_t pDT ) {
    mVel.x = mDX * paddle_t::MOVE_VEL;
    mVel.y = mDY * paddle_t::MOVE_VEL;

    mBBox.mPos += static_cast<float32_t>( pDT ) * mVel;
}


void paddle_t::irender() const {
    glBegin( GL_QUADS ); {
        glColor4ubv( &mColor[0] );
        glVertex2f( 0.0f, 0.0f );
        glVertex2f( 1.0f, 0.0f );
        glVertex2f( 1.0f, 1.0f );
        glVertex2f( 0.0f, 1.0f );
    } glEnd();
};


}