#include <sstream>

#include <glm/common.hpp>
#include <glm/ext/vector_float2.hpp>
#include <SDL2/SDL_opengl.h>

#include "hmp_paddle_t.h"

namespace hmp {

/// Class Functions ///

paddle_t::paddle_t( const box_t& pBBox ) :
        entity_t( pBBox ), mVel( 0.0f ), mAX( 0 ), mAY( 0 ) {
    
}


void paddle_t::accelerate( const int8_t pDX, const int8_t pDY ) {
    mAX = glm::clamp( mAX + pDX, -1, 1 );
    mAY = glm::clamp( mAY + pDY, -1, 1 );
}


void paddle_t::iupdate( const float64_t pDT ) {
    // TODO(JRC): Implement this function.
    // mBBox.mPos += pPos;
}


void paddle_t::irender() const {
    glBegin( GL_QUADS ); {
        // TODO(JRC): Add color.
        // glColor4ub( pColor[0], pColor[1], pColor[2], pColor[3] );
        glVertex2f( 0.0f, 0.0f );
        glVertex2f( 1.0f, 0.0f );
        glVertex2f( 1.0f, 1.0f );
        glVertex2f( 0.0f, 1.0f );
    } glEnd();
};

}
