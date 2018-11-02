#include <cstring>
#include <cstdlib>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "hmp.h"

LLCE_DYLOAD_API void init( hmp::state_t* pState, hmp::input_t* pInput ) {
    pState->time = 0.0;

    uint8_t playerColor[4] = { 0x00, 0xFF, 0xFF, 0xFF };
    pState->playerBox = hmp::box_t( -0.05f, -0.05f, 0.1f, 0.1f );
    std::memcpy( pState->playerColor, playerColor, sizeof(playerColor) );

    uint8_t boundsColor[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
    pState->boundsBox = hmp::box_t( -1.0f, -1.0f, 2.0f, 2.0f );
    std::memcpy( pState->boundsColor, boundsColor, sizeof(boundsColor) );
}


LLCE_DYLOAD_API void update( hmp::state_t* pState, hmp::input_t* pInput ) {
    // Process Input //

    float32_t dx = 0.0f, dy = 0.0f;
    if( pInput->keys[SDL_SCANCODE_W] ) {
        dy += 0.1;
    } if( pInput->keys[SDL_SCANCODE_S] ) {
        dy -= 0.1;
    } if( pInput->keys[SDL_SCANCODE_A] ) {
        dx -= 0.1;
    } if( pInput->keys[SDL_SCANCODE_D] ) {
        dx += 0.1;
    }

    // Update State //

    pState->playerBox.update( dx, dy );
    if( !pState->boundsBox.contains(pState->playerBox) ) {
        pState->playerBox.update( -dx, -dy );
    }
}


LLCE_DYLOAD_API void render( const hmp::state_t* pState, const hmp::input_t* pInput ) {
    pState->boundsBox.render( &(pState->boundsColor)[0] );
    pState->playerBox.render( &(pState->playerColor)[0] );
}
