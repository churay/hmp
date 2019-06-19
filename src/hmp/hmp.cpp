#include <cstring>
#include <cstdlib>

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
#include "hmp_data.h"
#include "hmp.h"

/// Global Declataions ///

typedef void (*init_f)( hmp::state_t* );
typedef void (*update_f)( hmp::state_t*, hmp::input_t*, const float64_t );
typedef void (*render_f)( const hmp::state_t*, const hmp::input_t*, const hmp::graphics_t* );

/// Per-Mode Functions ///

// Initialize Functions //

void init_game( hmp::state_t* pState ) {
    pState->rt = 0.0;
    pState->roundStarted = false;
    pState->roundServer = hmp::team::east;

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
    const hmp::scoreboard_t scoreEnt( hmp::box_t(scoreBasePos, scoreDims) );
    std::memcpy( (void*)&pState->scoreEnt, (void*)&scoreEnt, sizeof(hmp::scoreboard_t) );

    const glm::vec2 ballDims( 2.5e-2f, 2.5e-2f );
    const glm::vec2 ballPos = glm::vec2( 0.5f, 0.5f ) - 0.5f * ballDims;
    const hmp::ball_t ballEnt( hmp::box_t(ballPos, ballDims) );
    std::memcpy( (void*)&pState->ballEnt, (void*)&ballEnt, sizeof(hmp::ball_t) );

    const glm::vec2 paddleDims( 2.5e-2f, 1.0e-1f );

    const glm::vec2 westPos = glm::vec2( 2.0f * paddleDims[0], 0.5f - 0.5f * paddleDims[1] );
    const hmp::paddle_t westEnt( hmp::box_t(westPos, paddleDims), hmp::team::west );
    std::memcpy( (void*)&pState->paddleEnts[0], (void*)&westEnt, sizeof(hmp::paddle_t) );

    const glm::vec2 eastPos = glm::vec2( 1.0f - 3.0f * paddleDims[0], 0.5f - 0.5f * paddleDims[1] );
    const hmp::paddle_t eastEnt( hmp::box_t(eastPos, paddleDims), hmp::team::east );
    std::memcpy( (void*)&pState->paddleEnts[1], (void*)&eastEnt, sizeof(hmp::paddle_t) );
}


void init_menu( hmp::state_t* pState ) {
    pState->menuIdx = 0;
}


void init_pause( hmp::state_t* pState ) {
    // TODO(JRC)
}


void init_restart( hmp::state_t* pState ) {
    // TODO(JRC)
}

// Update Functions //

void update_game( hmp::state_t* pState, hmp::input_t* pInput, const float64_t pDT ) {
    // Process Inputs //

    int32_t dx[2] = { 0, 0 }, dy[2] = { 0, 0 };

    if( llce::input::isKeyDown(pInput->keyboard, SDL_SCANCODE_W) ) {
        dy[0] += 1;
    } if( llce::input::isKeyDown(pInput->keyboard, SDL_SCANCODE_S) ) {
        dy[0] -= 1;
    } if( llce::input::isKeyDown(pInput->keyboard, SDL_SCANCODE_A) ) {
        dx[0] -= 1;
    } if( llce::input::isKeyDown(pInput->keyboard, SDL_SCANCODE_D) ) {
        dx[0] += 1;
    } if( llce::input::isKeyDown(pInput->keyboard, SDL_SCANCODE_I) ) {
        dy[1] += 1;
    } if( llce::input::isKeyDown(pInput->keyboard, SDL_SCANCODE_K) ) {
        dy[1] -= 1;
    } if( llce::input::isKeyDown(pInput->keyboard, SDL_SCANCODE_J) ) {
        dx[1] -= 1;
    } if( llce::input::isKeyDown(pInput->keyboard, SDL_SCANCODE_L) ) {
        dx[1] += 1;
    }

    // Update State //

    pState->rt += pDT;

    // TODO(JRC): Movement along the x-axis for paddles is currently disabled;
    // inclusion of this style of movement needs to be determined.
    for( uint8_t paddleIdx = 0; paddleIdx < 2; paddleIdx++ ) {
        pState->paddleEnts[paddleIdx].move( 0, dy[paddleIdx] );
    }

    for( uint32_t entityIdx = 0; pState->entities[entityIdx] != nullptr; entityIdx++ ) {
        pState->entities[entityIdx]->update( pState->dt );
    }

    if( !pState->roundStarted && glm::length(pState->ballEnt.mVel) == 0.0f ) {
        const int8_t roundDir = ( pState->roundServer == hmp::team::west ) ? -1 : 1;
        const float64_t ballMaxTheta = hmp::ball_t::MAX_RICOCHET_ANGLE;

        // NOTE(JRC): I'm not convinced that I've chosen the correct scale value
        // for the random number to bring it into a reasonable floating point range,
        // but slightly lower fidelity won't be too important at the scale of this sim.
        float64_t ballThetaSeed = ( pState->rng.next() % (1 << 16) ) / ( (1 << 16) + 0.0 );
        float64_t ballTheta = 2 * ballMaxTheta * ballThetaSeed - ballMaxTheta;
        pState->ballEnt.mVel = hmp::ball_t::HINT_VEL *
            glm::vec2( roundDir * glm::cos(ballTheta), glm::sin(ballTheta) );
    } if( !pState->roundStarted && pState->rt >= hmp::ROUND_START_TIME ) {
        pState->ballEnt.mVel *= hmp::ball_t::MOVE_VEL / glm::length( pState->ballEnt.mVel );
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
            bool8_t isWestScore = ballX.contains( boundsX.mMax );
            pState->scoreEnt.tally( isWestScore ? -1 : 0, isWestScore ? 0 : -1 );
            pState->roundServer = isWestScore ? hmp::team::east : hmp::team::west;

            ballEnt.mBBox.mPos = glm::vec2( 0.5f, 0.5f ) - 0.5f * ballEnt.mBBox.mDims;
            ballEnt.mVel = glm::vec2( 0.0f, 0.0f );
            ballEnt.change( hmp::team::team_e::neutral );

            pState->rt = 0.0f;
            pState->roundStarted = false;

            if( pState->scoreEnt.mScores[0] <= 0 || pState->scoreEnt.mScores[1] <= 0 ) {
                pState->pmode = hmp::mode::menu;
            }
        }
    }

    for( uint8_t paddleIdx = 0; paddleIdx < 2; paddleIdx++ ) {
        hmp::paddle_t& paddleEnt = pState->paddleEnts[paddleIdx];
        if( paddleEnt.mBBox.overlaps(ballEnt.mBBox) ) {
            ballEnt.ricochet( &paddleEnt );
            ballEnt.change( static_cast<hmp::team::team_e>(paddleEnt.mTeam) );
            ballEnt.mVel *= 1.1f;
        } if( !boundsEnt.mBBox.contains(paddleEnt.mBBox) ) {
            paddleEnt.mBBox.embed( boundsEnt.mBBox );
        }
    }
}


void update_menu( hmp::state_t* pState, hmp::input_t* pInput, const float64_t pDT ) {
    int32_t dy[2] = { 0, 0 };

    if( llce::input::isKeyPressed(pInput->keyboard, SDL_SCANCODE_W) ) {
        dy[0] += 1;
    } if( llce::input::isKeyPressed(pInput->keyboard, SDL_SCANCODE_S) ) {
        dy[0] -= 1;
    } if( llce::input::isKeyPressed(pInput->keyboard, SDL_SCANCODE_I) ) {
        dy[1] += 1;
    } if( llce::input::isKeyPressed(pInput->keyboard, SDL_SCANCODE_K) ) {
        dy[1] -= 1;
    }

    pState->menuIdx = ( pState->menuIdx + dy[0] + dy[1] ) % hmp::MENU_ITEM_COUNT;
}


void update_pause( hmp::state_t* pState, hmp::input_t* pInput, const float64_t pDT ) {
    // TODO(JRC)
}


void update_restart( hmp::state_t* pState, hmp::input_t* pInput, const float64_t pDT ) {
    // TODO(JRC)
}

// Render Functions //

void render_game( const hmp::state_t* pState, const hmp::input_t* pInput, const hmp::graphics_t* pGraphics ) {
    { // Render State //
        hmp::gfx::fbo_context_t simFBOC(
            pGraphics->bufferFBOs[hmp::GFX_BUFFER_SIM],
            pGraphics->bufferRess[hmp::GFX_BUFFER_SIM] );

        for( uint32_t entityIdx = 0; pState->entities[entityIdx] != nullptr; entityIdx++ ) {
            pState->entities[entityIdx]->render();
        }
    }

    { // Render UI //
        hmp::gfx::fbo_context_t simFBOC(
            pGraphics->bufferFBOs[hmp::GFX_BUFFER_UI],
            pGraphics->bufferRess[hmp::GFX_BUFFER_UI] );

        pState->scoreEnt.render();
    }

    { // Render Master //
        const uint32_t masterFBO = pGraphics->bufferFBOs[hmp::GFX_BUFFER_MASTER];
        const vec2u32_t masterRes = pGraphics->bufferRess[hmp::GFX_BUFFER_MASTER];
        hmp::gfx::fbo_context_t masterFBOC( masterFBO, masterRes );

        hmp::gfx::render_context_t hmpRC( hmp::box_t(0.0f, 0.0f, 1.0f, 1.0f), &hmp::color::BACKGROUND );
        hmpRC.render();

        for( uint32_t bufferIdx = 0; bufferIdx < hmp::GFX_BUFFER_COUNT; bufferIdx++ ) {
            const uint32_t bufferFBO = pGraphics->bufferFBOs[bufferIdx];
            const vec2u32_t& bufferRes = pGraphics->bufferRess[bufferIdx];
            const hmp::box_t& bufferBox = pGraphics->bufferBoxs[bufferIdx];

            glBindFramebuffer( GL_READ_FRAMEBUFFER, bufferFBO );
            glBindFramebuffer( GL_DRAW_FRAMEBUFFER, masterFBO );
            for( uint32_t bufferTypeIdx = 0; bufferTypeIdx < 2; bufferTypeIdx++ ) {
                glBlitFramebuffer( 0, 0, bufferRes.x, bufferRes.y,
                    bufferBox.min().x * masterRes.x, bufferBox.min().y * masterRes.y,
                    bufferBox.max().x * masterRes.x, bufferBox.max().y * masterRes.y,
                    bufferTypeIdx ? GL_COLOR_BUFFER_BIT : GL_DEPTH_BUFFER_BIT,
                    bufferTypeIdx ? GL_LINEAR : GL_NEAREST );
            }
        }
    }
}


void render_menu( const hmp::state_t* pState, const hmp::input_t* pInput, const hmp::graphics_t* pGraphics ) {
    const uint32_t masterFBO = pGraphics->bufferFBOs[hmp::GFX_BUFFER_MASTER];
    const vec2u32_t masterRes = pGraphics->bufferRess[hmp::GFX_BUFFER_MASTER];
    hmp::gfx::fbo_context_t masterFBOC( masterFBO, masterRes );

    hmp::gfx::render_context_t hmpRC( hmp::box_t(0.0f, 0.0f, 1.0f, 1.0f), &hmp::color::BACKGROUND );
    hmpRC.render();

    { // Header //
        const float32_t cHeaderPadding = 0.05f;
        const vec2f32_t cHeaderDims = { 1.0f - 2.0f * cHeaderPadding, 0.25f };
        const vec2f32_t cHeaderPos = { cHeaderPadding, 1.0f - cHeaderPadding - cHeaderDims.y };

        hmp::gfx::render_context_t headerRC(
            hmp::box_t(cHeaderPos, cHeaderDims), &hmp::color::BACKGROUND );
        hmp::gfx::text::render( "HMP", &hmp::color::BACKGROUND2 );
    }

    { // Items //
        const float32_t cItemPadding = 0.05f;
        const vec2f32_t cItemDims = { 1.0f, 0.10f };
        const vec2f32_t cItemBase = { 0.0f, 0.50f };

        for( uint32_t itemIdx = 0; itemIdx < hmp::MENU_ITEM_COUNT; itemIdx++ ) {
            vec2f32_t itemPos = cItemBase -
                static_cast<float32_t>(itemIdx) * vec2f32_t( 0.0f, cItemDims.y + cItemPadding );
            hmp::gfx::render_context_t itemRC(
                hmp::box_t(itemPos, cItemDims, hmp::box_t::anchor_e::nw), &hmp::color::BACKGROUND2 );

            if( itemIdx == pState->menuIdx ) { itemRC.render(); }
            hmp::gfx::text::render( hmp::MENU_ITEM_TEXT[itemIdx], &hmp::color::TEAM[hmp::team::neutral] );
        }
    }
}


void render_pause( const hmp::state_t* pState, const hmp::input_t* pInput, const hmp::graphics_t* pGraphics ) {
    // TODO(JRC)
}


void render_restart( const hmp::state_t* pState, const hmp::input_t* pInput, const hmp::graphics_t* pGraphics ) {
    // TODO(JRC)
}

/// Per-Mode Tables ///

constexpr static init_f MODE_INIT_FUNS[] = { init_game, init_menu, init_pause, init_restart };
constexpr static update_f MODE_UPDATE_FUNS[] = { update_game, update_menu, update_pause, update_restart };
constexpr static render_f MODE_RENDER_FUNS[] = { render_game, render_menu, render_pause, render_restart };
constexpr static uint32_t MODE_COUNT = ARRAY_LEN( MODE_INIT_FUNS );

/// Interface Functions ///

extern "C" void boot( hmp::graphics_t* pGraphics ) {
    // Initialize Graphics //

    vec2u32_t* buffRess = &pGraphics->bufferRess[0];
    hmp::box_t* buffBoxs = &pGraphics->bufferBoxs[0];

    buffBoxs[hmp::GFX_BUFFER_MASTER] = hmp::box_t( 0.0f, 0.0f, 1.0f, 1.0f );
    buffBoxs[hmp::GFX_BUFFER_SIM] = hmp::box_t( 0.0f, 0.0f, 1.0f, 0.85f );
    buffBoxs[hmp::GFX_BUFFER_UI] = hmp::box_t( 0.0f, 0.85f, 1.0f, 0.15f );

    // NOTE(JRC): The following code ensures that buffers have consistent aspect
    // ratios relative to their output spaces in screen space. This fact is crucial
    // in making code work in 'hmp::gfx' related to fixing aspect ratios.
    buffRess[hmp::GFX_BUFFER_MASTER] = { 512, 512 };
    for( uint32_t bufferIdx = hmp::GFX_BUFFER_MASTER + 1; bufferIdx < hmp::GFX_BUFFER_COUNT; bufferIdx++ ) {
        buffRess[bufferIdx] = {
            (buffBoxs[bufferIdx].mDims.x / buffBoxs[hmp::GFX_BUFFER_MASTER].mDims.x) * buffRess[hmp::GFX_BUFFER_MASTER].x,
            (buffBoxs[bufferIdx].mDims.y / buffBoxs[hmp::GFX_BUFFER_MASTER].mDims.y) * buffRess[hmp::GFX_BUFFER_MASTER].y
        };
    }

    for( uint32_t bufferIdx = 0; bufferIdx < hmp::GFX_BUFFER_COUNT; bufferIdx++ ) {
        uint32_t& bufferFBO = pGraphics->bufferFBOs[bufferIdx];
        uint32_t& bufferTID = pGraphics->bufferTIDs[bufferIdx];
        uint32_t& bufferDID = pGraphics->bufferDIDs[bufferIdx];
        const vec2u32_t& bufferRes = pGraphics->bufferRess[bufferIdx];

        glGenFramebuffers( 1, &bufferFBO );
        glBindFramebuffer( GL_FRAMEBUFFER, bufferFBO );

        glGenTextures( 1, &bufferTID );
        glBindTexture( GL_TEXTURE_2D, bufferTID );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, bufferRes.x, bufferRes.y,
            0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, nullptr );
        glFramebufferTexture2D( GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferTID, 0 );

        glGenTextures( 1, &bufferDID );
        glBindTexture( GL_TEXTURE_2D, bufferDID );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, bufferRes.x, bufferRes.y,
            0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr );
        glFramebufferTexture2D( GL_FRAMEBUFFER,
            GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, bufferDID, 0 );

        LLCE_ASSERT_ERROR(
            glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
            "Failed to initialize HMP frame buffer " << bufferIdx << "; " <<
            "failed with frame buffer error " << glCheckFramebufferStatus(GL_FRAMEBUFFER) << "." );
    }

    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    glBindTexture( GL_TEXTURE_2D, 0 );
}


extern "C" void init( hmp::state_t* pState, hmp::input_t* pInput ) {
    // Initialize Global Variables //

    pState->dt = 0.0;
    pState->tt = 0.0;

    pState->mode = hmp::mode::boot;
    pState->pmode = hmp::mode::game;

    pState->rng = hmp::rng_t( hmp::RNG_SEED );

    // Initialize Input //

    std::memset( pInput, 0, sizeof(hmp::input_t) );

    // Initialize Per-Mode Variables //

    for( uint32_t modeIdx = 0; modeIdx < MODE_COUNT; modeIdx++ ) {
        MODE_INIT_FUNS[modeIdx]( pState );
    }
}


extern "C" void update( hmp::state_t* pState, hmp::input_t* pInput, const float64_t pDT ) {
    if( pState->mode != pState->pmode ) {
        MODE_INIT_FUNS[pState->pmode]( pState );
        pState->mode = pState->pmode;
    }

    pState->dt = pDT;
    pState->tt += pDT;

    MODE_UPDATE_FUNS[pState->mode]( pState, pInput, pDT );
}


extern "C" void render( const hmp::state_t* pState, const hmp::input_t* pInput, const hmp::graphics_t* pGraphics ) {
    hmp::gfx::render_context_t hmpRC( hmp::box_t(-1.0f, -1.0f, 2.0f, 2.0f), &hmp::color::BACKGROUND );
    hmpRC.render();

    MODE_RENDER_FUNS[pState->mode]( pState, pInput, pGraphics );
}
