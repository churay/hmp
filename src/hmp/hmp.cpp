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
    pState->entities[1] = &pState->ballEnt;
    pState->entities[1] = &pState->paddleEnts[0];
    pState->entities[2] = &pState->paddleEnts[1];

    // NOTE(JRC): For the virtual types below, a memory copy needs to be performed
    // instead of invoking the copy constructor in order to ensure that the v-table
    // is copied to the state entity, which is initialized to 'null' by default.

    const glm::vec2 boundsPos( 0.0f, 0.0f ), boundsDims( 1.0f, 1.0f );
    const hmp::bounds_t boundsEnt( hmp::box_t(boundsPos, boundsDims) );
    std::memcpy( (void*)&pState->boundsEnt, (void*)&boundsEnt, sizeof(hmp::bounds_t) );

    // TODO(JRC): Change 'ballEnt' to a 'hmp::ball_t' type.
    const uint8_t ballColor[4] = { 0x80, 0x7e, 0x76, 0xFF };
    const glm::vec2 ballDims( 2.5e-2f, 2.5e-2f );
    const glm::vec2 ballPos = glm::vec2( 0.5f, 0.5f ) - 0.5f * ballDims;
    // const hmp::ball_t ballEnt( hmp::box_t(ballPos, ballDims), &ballColor[0] );
    // std::memcpy( (void*)&pState->ballEnt, (void*)&ballEnt, sizeof(hmp::ball_t) );

    const glm::vec2 paddleDims( 2.5e-2f, 1.0e-1f );

    const uint8_t westColor[4] = { 0x9a, 0x86, 0x00, 0xFF };
    const glm::vec2 westPos = glm::vec2( 2.0f * paddleDims[0], 0.5f - 0.5f * paddleDims[1] );
    const hmp::paddle_t westEnt( hmp::box_t(westPos, paddleDims), &westColor[0] );
    std::memcpy( (void*)&pState->paddleEnts[0], (void*)&westEnt, sizeof(hmp::paddle_t) );

    const uint8_t eastColor[4] = { 0x00, 0x9d, 0xa3, 0xFF };
    const glm::vec2 eastPos = glm::vec2( 1.0f - 3.0f * paddleDims[0], 0.5f - 0.5f * paddleDims[1] );
    const hmp::paddle_t eastEnt( hmp::box_t(eastPos, paddleDims), &eastColor[0] );
    std::memcpy( (void*)&pState->paddleEnts[1], (void*)&eastEnt, sizeof(hmp::paddle_t) );
}


LLCE_DYLOAD_API void update( hmp::state_t* pState, hmp::input_t* pInput ) {
    // Process Input //

    int32_t dx[2] = { 0, 0 }, dy[2] = { 0, 0 };
    if( pInput->keys[SDL_SCANCODE_W] ) {
        dy[0] += 1;
    } if( pInput->keys[SDL_SCANCODE_S] ) {
        dy[0] -= 1;
    } if( pInput->keys[SDL_SCANCODE_A] ) {
        dx[0] -= 1;
    } if( pInput->keys[SDL_SCANCODE_D] ) {
        dx[0] += 1;
    } if( pInput->keys[SDL_SCANCODE_I] ) {
        dy[1] += 1;
    } if( pInput->keys[SDL_SCANCODE_K] ) {
        dy[1] -= 1;
    } if( pInput->keys[SDL_SCANCODE_J] ) {
        dx[1] -= 1;
    } if( pInput->keys[SDL_SCANCODE_L] ) {
        dx[1] += 1;
    }

    // TODO(JRC): Movement along the x-axis for paddles is currently disabled;
    // inclusion of this style of movement needs to be determined.
    for( uint8_t paddleIdx = 0; paddleIdx < 2; paddleIdx++ ) {
        pState->paddleEnts[paddleIdx].move( 0, dy[paddleIdx] );
    }

    // Update State //

    for( uint32_t entityIdx = 0; pState->entities[entityIdx] != nullptr; entityIdx++ ) {
        pState->entities[entityIdx]->update( pState->dt );
    }

    // Resolve State(?) //

    // TODO(JRC): There should be some more universal handler for collision
    // detection/resolution.
    for( uint8_t paddleIdx = 0; paddleIdx < 2; paddleIdx++ ) {
        hmp::paddle_t& paddleEnt = pState->paddleEnts[paddleIdx];
        if( !pState->boundsEnt.mBBox.contains(paddleEnt.mBBox) ) {
            paddleEnt.mBBox.embed( pState->boundsEnt.mBBox );
        }
    }
}


LLCE_DYLOAD_API void render( const hmp::state_t* pState, const hmp::input_t* pInput ) {
    // Render State //

    for( uint32_t entityIdx = 0; pState->entities[entityIdx] != nullptr; entityIdx++ ) {
        pState->entities[entityIdx]->render();
    }
}
