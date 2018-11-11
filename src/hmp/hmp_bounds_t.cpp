#include <cstring>
#include <sstream>

#include <glm/common.hpp>
#include <glm/ext/vector_float2.hpp>
#include <SDL2/SDL_opengl.h>

#include "hmp_bounds_t.h"

namespace hmp {

/// Class Functions ///

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

}
