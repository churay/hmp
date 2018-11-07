#include <sstream>

#include <SDL2/SDL_opengl.h>

#include "hmp_paddle_t.h"

namespace hmp {

/// Class Functions ///

entity_t::entity_t( const box_t& pBBox ) : mBBox( pBBox ) {
    
}


void entity_t::update( const float64_t pDT ) {
    mLifetime += pDT;
    iupdate( pDT );
}


void entity_t::render() const {
    // TODO(JRC): Set up the render matrix such that the internal render
    // function can render its contents to the space (1.0, 1.0).

    glPushMatrix();
    irender();
    glPopMatrix();
}

}
