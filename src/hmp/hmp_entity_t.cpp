#include <cstring>

#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <SDL2/SDL_opengl.h>

#include "hmp_gfx.h"
#include "hmp_entity_t.h"

namespace hmp {

/// Class Functions ///

entity_t::entity_t( const box_t& pBBox, const color_t* pColor ) :
        mBBox( pBBox ), mVel( 0.0f, 0.0f ), mColor( pColor ), mLifetime( 0.0f ) {
    
}


entity_t::~entity_t() {
    
}


void entity_t::update( const float64_t pDT ) {
    mLifetime += pDT;
    iupdate( pDT );
}


void entity_t::render() const {
    hmp::gfx::render_context_t entityRC( mBBox, mColor );
    irender();
}


void entity_t::iupdate( const float64_t pDT ) {
    mBBox.mPos += static_cast<float32_t>( pDT ) * mVel;
}


void entity_t::irender() const {
    glBegin( GL_QUADS ); {
        glVertex2f( 0.0f, 0.0f );
        glVertex2f( 1.0f, 0.0f );
        glVertex2f( 1.0f, 1.0f );
        glVertex2f( 0.0f, 1.0f );
    } glEnd();
}

}
