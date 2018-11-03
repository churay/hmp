#include <cstring>
#include <cstdlib>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <glm/ext/vector_float2.hpp>

#include "hmp.h"

LLCE_DYLOAD_API void init( hmp::state_t* pState, hmp::input_t* pInput ) {
    pState->time = 0.0;

    uint8_t playerColor[4] = { 0x00, 0xFF, 0xFF, 0xFF };
    std::memcpy( pState->playerColor, playerColor, sizeof(playerColor) );
    pState->playerBox = hmp::box_t( glm::vec2(-0.05f, -0.05f), glm::vec2(0.1f, 0.1f) );

    uint8_t boundsColor[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
    std::memcpy( pState->boundsColor, boundsColor, sizeof(boundsColor) );
    pState->boundsBox = hmp::box_t( glm::vec2(-1.0f, -1.0f), glm::vec2(2.0f, 2.0f) );
}


LLCE_DYLOAD_API void update( hmp::state_t* pState, hmp::input_t* pInput ) {
    // Process Input //

    glm::vec2 dp( 0.0f, 0.0f );
    if( pInput->keys[SDL_SCANCODE_W] ) {
        dp.y += 0.1f;
    } if( pInput->keys[SDL_SCANCODE_S] ) {
        dp.y -= 0.1f;
    } if( pInput->keys[SDL_SCANCODE_A] ) {
        dp.x -= 0.1f;
    } if( pInput->keys[SDL_SCANCODE_D] ) {
        dp.x += 0.1f;
    }

    // Update State //

    pState->playerBox.update( dp );
    if( !pState->boundsBox.contains(pState->playerBox) ) {
        pState->playerBox.update( -dp );
    }
}


LLCE_DYLOAD_API void render( const hmp::state_t* pState, const hmp::input_t* pInput ) {
    pState->boundsBox.render( &(pState->boundsColor)[0] );
    pState->playerBox.render( &(pState->playerColor)[0] );
}
