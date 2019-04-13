#include <cstring>
#include <cstdlib>

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

#include "hmp.h"


extern "C" void init( hmp::state_t* pState, hmp::input_t* pInput ) {
    // Initialize Simulation //

    pState->dt = 0.0; // frame delta time
    pState->rt = 0.0; // round time
    pState->tt = 0.0; // total time
    pState->roundStarted = false;

    // Initialize Graphics //

    uint32_t* stateBufferGLIDs[] = { &pState->simBufferGLID, &pState->uiBufferGLID };
    uint32_t* stateTextureGLIDs[] = { &pState->simTextureGLID, &pState->uiTextureGLID };
    const uint32_t* stateBufferResolutions[] = { &hmp::SIM_RESOLUTION[0], &hmp::UI_RESOLUTION[0] };

    const uint32_t cStateBufferCount = ARRAY_LEN( stateBufferGLIDs );
    for( uint32_t bufferIdx = 0; bufferIdx < cStateBufferCount; bufferIdx++ ) {
        uint32_t& bufferGLID = *stateBufferGLIDs[bufferIdx];
        uint32_t& bufferTextureGLID = *stateTextureGLIDs[bufferIdx];

        glGenFramebuffers( 1, &bufferGLID );
        glBindFramebuffer( GL_FRAMEBUFFER, bufferGLID );

        glGenTextures( 1, &bufferTextureGLID );
        glBindTexture( GL_TEXTURE_2D, bufferTextureGLID );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8,
            stateBufferResolutions[bufferIdx][0], stateBufferResolutions[bufferIdx][1],
            0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, nullptr );
        glFramebufferTexture2D( GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferTextureGLID, 0 );

        LLCE_ASSERT_ERROR(
            glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
            "Failed to initialize HMP frame buffer " << bufferIdx << "." );
    }

    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    glBindTexture( GL_TEXTURE_2D, 0 );

    // Initialize Entities //

    // TODO(JRC): This should be more automated if possible (iterate over
    // the state variables after 'entities' array, perhaps?).
    uint32_t entityIdx = 0;
    pState->entities[entityIdx++] = &pState->boundsEnt;
    // pState->entities[entityIdx++] = &pState->scoreEnt;
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

    const glm::vec2 scoreBasePos( 0.0f, 0.0f ), scoreDims( 1.0f, 1.0f );
    const hmp::scoreboard_t scoreEnt( hmp::box_t(scoreBasePos, scoreDims), hmp::COLOR_WEST, hmp::COLOR_EAST );
    std::memcpy( (void*)&pState->scoreEnt, (void*)&scoreEnt, sizeof(hmp::scoreboard_t) );

    const glm::vec2 ballDims( 2.5e-2f, 2.5e-2f );
    const glm::vec2 ballPos = glm::vec2( 0.5f, 0.5f ) - 0.5f * ballDims;
    const hmp::ball_t ballEnt( hmp::box_t(ballPos, ballDims) );
    std::memcpy( (void*)&pState->ballEnt, (void*)&ballEnt, sizeof(hmp::ball_t) );

    const glm::vec2 paddleDims( 2.5e-2f, 1.0e-1f );

    const glm::vec2 westPos = glm::vec2( 2.0f * paddleDims[0], 0.5f - 0.5f * paddleDims[1] );
    const hmp::paddle_t westEnt( hmp::box_t(westPos, paddleDims), hmp::COLOR_WEST );
    std::memcpy( (void*)&pState->paddleEnts[0], (void*)&westEnt, sizeof(hmp::paddle_t) );

    const glm::vec2 eastPos = glm::vec2( 1.0f - 3.0f * paddleDims[0], 0.5f - 0.5f * paddleDims[1] );
    const hmp::paddle_t eastEnt( hmp::box_t(eastPos, paddleDims), hmp::COLOR_EAST );
    std::memcpy( (void*)&pState->paddleEnts[1], (void*)&eastEnt, sizeof(hmp::paddle_t) );

    std::memset( pInput->keys, 0, sizeof(pInput->keys) );
    std::memset( pInput->diffs, hmp::KEY_DIFF_NONE, sizeof(pInput->diffs) );
}


extern "C" void update( hmp::state_t* pState, hmp::input_t* pInput, const float64_t pDT ) {
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
            ballEnt.mVel *= 1.1f;
        } if( !boundsEnt.mBBox.contains(paddleEnt.mBBox) ) {
            paddleEnt.mBBox.embed( boundsEnt.mBBox );
        }
    }
}


extern "C" void render( const hmp::state_t* pState, const hmp::input_t* pInput ) {
    // Render State //

    glBindFramebuffer( GL_FRAMEBUFFER, pState->simBufferGLID );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glMatrixMode( GL_MODELVIEW );
    glPushMatrix(); {
        glLoadIdentity();
        glm::mat4 matWorldView( 1.0f );
        matWorldView *= glm::translate( glm::mat4(1.0f), glm::vec3(-1.0f, -1.0f, 0.0f) );
        matWorldView *= glm::scale( glm::mat4(1.0f), glm::vec3(0.8f, 1.0f, 0.0f) );
        glMultMatrixf( &matWorldView[0][0] );

        for( uint32_t entityIdx = 0; pState->entities[entityIdx] != nullptr; entityIdx++ ) {
            pState->entities[entityIdx]->render();
        }
    } glPopMatrix();

    glBindFramebuffer( GL_FRAMEBUFFER, 0 );

    glEnable( GL_TEXTURE_2D ); {
        glBindTexture( GL_TEXTURE_2D, pState->simTextureGLID );
        // NOTE(JRC): This is required to get the expected/correct texture color,
        // but it's unclear as to why. OpenGL may perform color mixing by default?
        glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
        glBegin( GL_QUADS ); {
            glTexCoord2f( 0.0f, 0.0f ); glVertex2f( 0.0f, 0.0f );
            glTexCoord2f( 0.0f, 1.0f ); glVertex2f( 0.0f, 0.8f );
            glTexCoord2f( 1.0f, 1.0f ); glVertex2f( 1.0f, 0.8f );
            glTexCoord2f( 1.0f, 0.0f ); glVertex2f( 1.0f, 0.0f );
        } glEnd();
        glBindTexture( GL_TEXTURE_2D, 0 );
    } glDisable( GL_TEXTURE_2D );

    // Render UI //

    // TODO(JRC): Add code to render the UI here.

    // glBegin( GL_QUADS ); {
    //     glColor4ub( 0xFF, 0xFF, 0xFF, 0xFF );
    //     glVertex2f( 0.0f, 0.8f );
    //     glVertex2f( 0.0f, 1.0f );
    //     glVertex2f( 1.0f, 1.0f );
    //     glVertex2f( 1.0f, 0.8f );
    // } glEnd();
}
