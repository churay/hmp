#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_opengl_glext.h>

#include <glm/geometric.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "hmp_gfx.h"

namespace hmp {

namespace gfx {

/// Namespace Functions ///

void render( const hmp::box_t& pBox, const color_t& pColor ) {
    // TODO(JRC): It would be ideal if this could somehow be combined with the
    // very similar implementation in the "hmp::entity_t::render()" function.
    glPushMatrix(); {
        glm::mat4 matModelWorld( 1.0f );
        matModelWorld *= glm::translate( glm::mat4(1.0f), glm::vec3(pBox.mPos.x, pBox.mPos.y, 0.0f) );
        matModelWorld *= glm::scale( glm::mat4(1.0f), glm::vec3(pBox.mDims.x, pBox.mDims.y, 0.0f) );
        glMultMatrixf( &matModelWorld[0][0] );

        glBegin( GL_QUADS ); {
            glColor4ubv( (uint8_t*)&pColor );
            glVertex2f( 0.0f, 0.0f );
            glVertex2f( 1.0f, 0.0f );
            glVertex2f( 1.0f, 1.0f );
            glVertex2f( 0.0f, 1.0f );
        } glEnd();
    } glPopMatrix();
}

};

};
