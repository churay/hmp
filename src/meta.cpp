#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_opengl_glext.h>

#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "meta.h"

namespace llce {

namespace meta {

/// 'llce::meta' Functions ///

bool32_t render( const hmp::state_t* pState, const hmp::input_t* pInput, const hmp::output_t* pOutput ) {
    glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
    glBegin( GL_QUADS ); {
        glVertex2f( +0.0f, +0.0f );
        glVertex2f( +0.0f, +1.0f );
        glVertex2f( +1.0f, +1.0f );
        glVertex2f( +1.0f, +0.0f );
    } glEnd();

    return true;
}

}

}
