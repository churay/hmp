#include <cstring>
#include <cstdlib>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <glm/geometric.hpp>
#include <glm/ext/vector_float2.hpp>

#include "hmp.h"

LLCE_DYLOAD_API void init( hmp::state_t* pState, hmp::input_t* pInput ) {
    pState->dt = 0.0; // frame delta time
    pState->rt = 0.0; // round time
    pState->tt = 0.0; // total time
    pState->roundStarted = false;

    uint32_t entityIdx = 0;
    pState->entities[entityIdx++] = &pState->boundsEnt;
    pState->entities[entityIdx++] = &pState->ballEnt;
    for( uint32_t paddleIdx = 0; paddleIdx < 2; paddleIdx++ ) {
        pState->entities[entityIdx++] = &pState->paddleEnts[paddleIdx];
    } for( ; entityIdx < hmp::MAX_ENTITIES; ) {
        pState->entities[entityIdx++] = nullptr;
    }

    // NOTE(JRC): For the virtual types below, a memory copy needs to be performed
    // instead of invoking the copy constructor in order to ensure that the v-table
    // is copied to the state entity, which is initialized to 'null' by default.

    const glm::vec2 boundsBasePos( 0.0f, 0.0f ), boundsDims( 1.0f, 1.0f );
    const hmp::bounds_t boundsEnt( hmp::box_t(boundsBasePos, boundsDims) );
    std::memcpy( (void*)&pState->boundsEnt, (void*)&boundsEnt, sizeof(hmp::bounds_t) );

    for( uint32_t ricochetIdx = 0; ricochetIdx < 2; ricochetIdx++ ) {
        const glm::vec2 boundsPos = boundsBasePos + glm::vec2( 0.0f, (ricochetIdx != 0) ? 1.0f : -1.0f );
        hmp::bounds_t ricochetEnt( hmp::box_t(boundsPos, boundsDims) );
        std::memcpy( (void*)&pState->ricochetEnts[ricochetIdx], (void*)&ricochetEnt, sizeof(hmp::bounds_t) );
    }

    const glm::vec2 ballDims( 2.5e-2f, 2.5e-2f );
    const glm::vec2 ballPos = glm::vec2( 0.5f, 0.5f ) - 0.5f * ballDims;
    const hmp::ball_t ballEnt( hmp::box_t(ballPos, ballDims) );
    std::memcpy( (void*)&pState->ballEnt, (void*)&ballEnt, sizeof(hmp::ball_t) );

    const glm::vec2 paddleDims( 2.5e-2f, 1.0e-1f );

    const color_t westColor = { 0x9a, 0x86, 0x00, 0xFF };
    const glm::vec2 westPos = glm::vec2( 2.0f * paddleDims[0], 0.5f - 0.5f * paddleDims[1] );
    const hmp::paddle_t westEnt( hmp::box_t(westPos, paddleDims), westColor );
    std::memcpy( (void*)&pState->paddleEnts[0], (void*)&westEnt, sizeof(hmp::paddle_t) );

    const color_t eastColor = { 0x00, 0x9d, 0xa3, 0xFF };
    const glm::vec2 eastPos = glm::vec2( 1.0f - 3.0f * paddleDims[0], 0.5f - 0.5f * paddleDims[1] );
    const hmp::paddle_t eastEnt( hmp::box_t(eastPos, paddleDims), eastColor );
    std::memcpy( (void*)&pState->paddleEnts[1], (void*)&eastEnt, sizeof(hmp::paddle_t) );
}


LLCE_DYLOAD_API void update( hmp::state_t* pState, hmp::input_t* pInput, const float64_t pDT ) {
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

    pState->dt = pDT;
    pState->rt += pDT;
    pState->tt += pDT;

    for( uint32_t entityIdx = 0; pState->entities[entityIdx] != nullptr; entityIdx++ ) {
        pState->entities[entityIdx]->update( pState->dt );
    }

    if( !pState->roundStarted && pState->rt >= hmp::ROUND_START_TIME ) {
        // TODO(JRC): Randomize this vector.
        pState->ballEnt.mVel = glm::normalize( glm::vec2(1.0f, 1.0f) ) * hmp::ball_t::MOVE_VEL;
        pState->roundStarted = true;
    }

    // Resolve Collisions //

    hmp::bounds_t& boundsEnt = pState->boundsEnt;
    hmp::ball_t& ballEnt = pState->ballEnt;

    if( !boundsEnt.mBBox.contains(ballEnt.mBBox) ) {
        hmp::interval_t ballX = ballEnt.mBBox.xbounds(), ballY = ballEnt.mBBox.ybounds();
        hmp::interval_t boundsX = boundsEnt.mBBox.xbounds(), boundsY = boundsEnt.mBBox.ybounds();
        if( !boundsY.contains(ballY) ) {
            uint8_t ricochetIdx = (uint8_t)( ballY.mMax > boundsY.mMax );
            ballEnt.ricochet( &pState->ricochetEnts[ricochetIdx] );
        } if( !boundsX.contains(ballX) ) {
            ballEnt.mBBox.mPos = glm::vec2( 0.5f, 0.5f ) - 0.5f * ballEnt.mBBox.mDims;
            ballEnt.mVel = glm::vec2( 0.0f, 0.0f );
            pState->rt = 0.0f;
            pState->roundStarted = false;
        }
    }

    for( uint8_t paddleIdx = 0; paddleIdx < 2; paddleIdx++ ) {
        hmp::paddle_t& paddleEnt = pState->paddleEnts[paddleIdx];
        if( paddleEnt.mBBox.overlaps(ballEnt.mBBox) ) {
            ballEnt.ricochet( &paddleEnt );
            // TODO(JRC): Increase magnitude of vector.
            ballEnt.mVel *= 1.1f;
        } if( !boundsEnt.mBBox.contains(paddleEnt.mBBox) ) {
            paddleEnt.mBBox.embed( boundsEnt.mBBox );
        }
    }
}


LLCE_DYLOAD_API void render( const hmp::state_t* pState, const hmp::input_t* pInput ) {
    // Render State //

    for( uint32_t entityIdx = 0; pState->entities[entityIdx] != nullptr; entityIdx++ ) {
        pState->entities[entityIdx]->render();
    }

    // Render UI //

    glBegin( GL_QUADS ); {
        glColor4ubv( (uint8_t*)&hmp::UI_COLOR );
    } glEnd();
}
