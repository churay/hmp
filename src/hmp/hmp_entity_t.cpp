#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>

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
    glPushMatrix(); {
        glm::mat4 matModelWorld( 1.0f );
        matModelWorld *= glm::translate( glm::mat4(1.0f), glm::vec3(mBBox.mPos.x, mBBox.mPos.y, 0.0f) );
        matModelWorld *= glm::scale( glm::mat4(1.0f), glm::vec3(mBBox.mDims.x, mBBox.mDims.y, 0.0f) );
        glMultMatrixf( &matModelWorld[0][0] );
        irender();
    } glPopMatrix();
}

}
