#include <cstring>

#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <SDL2/SDL_opengl.h>

#include "hmp_entity_t.h"

namespace hmp {

/// Class Functions ///

entity_t::entity_t( const box_t& pBBox, const color_t& pColor ) :
        mBBox( pBBox ), mColor( pColor ), mLifetime( 0.0f ) {
}


void entity_t::update( const float64_t pDT ) {
    mLifetime += pDT;
    iupdate( pDT );
}


void entity_t::render() const {
    glPushMatrix(); {
        glm::mat4 matModelWorld( 1.0f );
        matModelWorld *= glm::translate( glm::mat4(1.0f), glm::vec3(mBBox.mPos.x, mBBox.mPos.y, 0.0f) );
        matModelWorld *= glm::scale( glm::mat4(1.0f), glm::vec3(mBBox.mDims.x, mBBox.mDims.y, 0.0f) );
        glMultMatrixf( &matModelWorld[0][0] );
        irender();
    } glPopMatrix();
}


void entity_t::irender() const {
    glBegin( GL_QUADS ); {
        glColor4ubv( (uint8_t*)&mColor );
        glVertex2f( 0.0f, 0.0f );
        glVertex2f( 1.0f, 0.0f );
        glVertex2f( 1.0f, 1.0f );
        glVertex2f( 0.0f, 1.0f );
    } glEnd();
}

}
