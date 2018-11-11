#include <cstring>
#include <cstdlib>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <glm/ext/vector_float2.hpp>

#include "hmp.h"

LLCE_DYLOAD_API void init( hmp::state_t* pState, hmp::input_t* pInput ) {
    pState->dt = 0.0;
    pState->tt = 0.0;

    for( uint32_t entityIdx = 0; entityIdx < hmp::MAX_ENTITIES; entityIdx++ ) {
        pState->entities[entityIdx] = nullptr;
    }

    pState->entities[0] = &pState->boundsEnt;
    pState->entities[1] = &pState->playerEnt;

    // NOTE(JRC): For the virtual types below, a memory copy needs to be performed
    // instead of invoking the copy constructor in order to ensure that the v-table
    // is copied to the state entity, which is initialized to 'null' by default.

    const glm::vec2 boundsPos( 0.0f, 0.0f ), boundsDims( 1.0f, 1.0f );
    const hmp::bounds_t boundsEnt( hmp::box_t(boundsPos, boundsDims) );
    std::memcpy( (void*)&pState->boundsEnt, (void*)&boundsEnt, sizeof(hmp::bounds_t) );

    const uint8_t playerColor[4] = { 0x80, 0x7e, 0x76, 0xFF };
    const glm::vec2 playerDims( 0.1f, 0.2f );
    const glm::vec2 playerPos = glm::vec2( 0.5f, 0.5f ) - 0.5f * playerDims;
    const hmp::paddle_t playerEnt( hmp::box_t(playerPos, playerDims), &playerColor[0] );
    std::memcpy( (void*)&pState->playerEnt, (void*)&playerEnt, sizeof(hmp::paddle_t) );
}


LLCE_DYLOAD_API void update( hmp::state_t* pState, hmp::input_t* pInput ) {
    // Process Input //

    int32_t dx = 0, dy = 0;
    if( pInput->keys[SDL_SCANCODE_W] ) {
        dy += 1;
    } if( pInput->keys[SDL_SCANCODE_S] ) {
        dy -= 1;
    } if( pInput->keys[SDL_SCANCODE_A] ) {
        dx -= 1;
    } if( pInput->keys[SDL_SCANCODE_D] ) {
        dx += 1;
    }

    // TODO(JRC): Movement along the x-axis for paddles is currently disabled;
    // inclusion of this style of movement needs to be determined.
    pState->playerEnt.move( 0, dy );

    // Update State //

    for( uint32_t entityIdx = 0; pState->entities[entityIdx] != nullptr; entityIdx++ ) {
        pState->entities[entityIdx]->update( pState->dt );
    }

    // Resolve State(?) //

    // TODO(JRC): There should be some more universal handler for collision
    // detection/resolution.
    if( !pState->boundsEnt.mBBox.contains(pState->playerEnt.mBBox) ) {
        pState->playerEnt.mBBox.embed( pState->boundsEnt.mBBox );
    }
}


LLCE_DYLOAD_API void render( const hmp::state_t* pState, const hmp::input_t* pInput ) {
    // Render State //

    for( uint32_t entityIdx = 0; pState->entities[entityIdx] != nullptr; entityIdx++ ) {
        pState->entities[entityIdx]->render();
    }
}
