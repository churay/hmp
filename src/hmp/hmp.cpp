#include <cstring>
#include <cstdlib>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <glm/ext/vector_float2.hpp>

#include "hmp.h"

LLCE_DYLOAD_API void init( hmp::state_t* pState, hmp::input_t* pInput ) {
    pState->dt = 0.0;
    pState->tt = 0.0;

    const uint8_t boundsColor[4] = { 0x00, 0x2b, 0x36, 0xFF };
    std::memcpy( pState->boundsColor, boundsColor, sizeof(boundsColor) );
    const glm::vec2 boundsPos( 0.0f, 0.0f ), boundsDims( 1.0f, 1.0f );
    pState->boundsBox = hmp::box_t( boundsPos, boundsDims );

    const glm::vec2 playerDims( 0.1f, 0.1f );
    const glm::vec2 playerPos = glm::vec2( 0.5f, 0.5f ) - 0.5f * playerDims;
    pState->playerEnt = hmp::paddle_t( hmp::box_t(playerPos, playerDims) );
}


LLCE_DYLOAD_API void update( hmp::state_t* pState, hmp::input_t* pInput ) {
    // Process Input //

    int32_t dx = 0, dy = 0;
    if( pInput->keys[SDL_SCANCODE_W] ) {
        dy += 1;
    } if( pInput->keys[SDL_SCANCODE_S] ) {
        dy -= 1;
    } if( pInput->keys[SDL_SCANCODE_A] ) {
        dx += 1;
    } if( pInput->keys[SDL_SCANCODE_D] ) {
        dx -= 1;
    }

    pState->playerEnt.move( dx, dy );

    // Update State //

    hmp::box_t prevPlayerBox = pState->playerEnt.mBBox;
    pState->playerEnt.update( pState->dt );
    if( !pState->boundsBox.contains(pState->playerEnt.mBBox) ) {
        pState->playerEnt.mBBox = prevPlayerBox;
    }
}


LLCE_DYLOAD_API void render( const hmp::state_t* pState, const hmp::input_t* pInput ) {
    // TODO(JRC): Improve rendering mechanism for the bounding box?
    pState->boundsBox.render( &(pState->boundsColor)[0] );
    glBegin( GL_QUADS ); {
        glColor4ub( pState->boundsColor[0], pState->boundsColor[1], pState->boundsColor[2], pState->boundsColor[3] );
        glVertex2f( 0.0f, 0.0f );
        glVertex2f( 1.0f, 0.0f );
        glVertex2f( 1.0f, 1.0f );
        glVertex2f( 0.0f, 1.0f );
    } glEnd();

    pState->playerEnt.render();
}
