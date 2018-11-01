#include <cstdlib>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "hmplib.h"

LLCE_DYLOAD_API void update( hmplib::state* pState, hmplib::input* pInput ) {
    if( pInput->keys[SDL_SCANCODE_W] ) {
        pState->box[1] += 0.1;
    } if( pInput->keys[SDL_SCANCODE_S] ) {
        pState->box[1] -= 0.1;
    } if( pInput->keys[SDL_SCANCODE_A] ) {
        pState->box[0] -= 0.1;
    } if( pInput->keys[SDL_SCANCODE_D] ) {
        pState->box[0] += 0.1;
    }

    pState->box[0] = std::min( 1.0f - pState->box[2], std::max(-1.0f, pState->box[0]) );
    pState->box[1] = std::min( 1.0f - pState->box[3], std::max(-1.0f, pState->box[1]) );
}


LLCE_DYLOAD_API void render( const hmplib::state* pState, const hmplib::input* pInput ) {
    glBegin( GL_QUADS );
        glColor4ub( pState->backColor[0], pState->backColor[1], pState->backColor[2], pState->backColor[3] );
        glVertex2f( -1.0f, -1.0f );
        glVertex2f( +1.0f, -1.0f );
        glVertex2f( +1.0f, +1.0f );
        glVertex2f( -1.0f, +1.0f );
    glEnd();

    glBegin( GL_QUADS );
        glColor4ub( pState->boxColor[0], pState->boxColor[1], pState->boxColor[2], pState->boxColor[3] );
        glVertex2f( pState->box[0] + 0.0 * pState->box[2], pState->box[1] + 0.0 * pState->box[3] );
        glVertex2f( pState->box[0] + 1.0 * pState->box[2], pState->box[1] + 0.0 * pState->box[3] );
        glVertex2f( pState->box[0] + 1.0 * pState->box[2], pState->box[1] + 1.0 * pState->box[3] );
        glVertex2f( pState->box[0] + 0.0 * pState->box[2], pState->box[1] + 1.0 * pState->box[3] );
    glEnd();
}
