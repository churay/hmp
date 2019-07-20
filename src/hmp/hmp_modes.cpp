#include <cstring>
#include <sstream>

#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_opengl_glext.h>

#include <glm/common.hpp>
#include <glm/geometric.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/ext/scalar_constants.hpp>

#include "hmp_modes.h"
#include "hmp_gfx.h"
#include "hmp_sfx.h"
#include "hmp_data.h"
#include "hmp_entities.h"

namespace hmp {

namespace mode {

/// 'hmp::mode::game' Functions  ///

bool32_t game::init( hmp::state_t* pState ) {
    pState->rt = 0.0;
    pState->roundStarted = false;
    pState->roundServer = hmp::team::east;

    const glm::vec2 boundsBasePos( 0.0f, 0.0f ), boundsDims( 1.0f, 1.0f );
    pState->boundsEnt = hmp::bounds_t( hmp::box_t(boundsBasePos, boundsDims) );

    for( uint32_t ricochetIdx = 0; ricochetIdx < 2; ricochetIdx++ ) {
        const glm::vec2 boundsPos = boundsBasePos + glm::vec2( 0.0f, (ricochetIdx != 0) ? 1.0f : -1.0f );
        pState->ricochetEnts[ricochetIdx] = hmp::bounds_t( hmp::box_t(boundsPos, boundsDims) );
    }

    const glm::vec2 scoreBasePos( 0.0f, 0.0f ), scoreDims( 1.0f, 1.0f );
    pState->scoreEnt = hmp::scoreboard_t( hmp::box_t(scoreBasePos, scoreDims) );

    const glm::vec2 ballDims( 2.5e-2f, 2.5e-2f );
    const glm::vec2 ballPos = glm::vec2( 0.5f, 0.5f ) - 0.5f * ballDims;
    pState->ballEnt = hmp::ball_t( hmp::box_t(ballPos, ballDims) );

    const glm::vec2 paddleDims( 2.5e-2f, 1.0e-1f );
    const glm::vec2 westPos = glm::vec2( 2.0f * paddleDims[0], 0.5f - 0.5f * paddleDims[1] );
    pState->paddleEnts[hmp::team::west] = hmp::paddle_t( hmp::box_t(westPos, paddleDims), hmp::team::west );
    const glm::vec2 eastPos = glm::vec2( 1.0f - 3.0f * paddleDims[0], 0.5f - 0.5f * paddleDims[1] );
    pState->paddleEnts[hmp::team::east] = hmp::paddle_t( hmp::box_t(eastPos, paddleDims), hmp::team::east );

    return true;
}


bool32_t game::update( hmp::state_t* pState, hmp::input_t* pInput, const float64_t pDT ) {
    const static hmp::sfx::waveform_t csPaddleRicochetSFX(
        hmp::sfx::wave::sine, hmp::sfx::MID_A_FREQ * 5.0, hmp::sfx::VOLUME, 0.0 );
    const static hmp::sfx::waveform_t csWallRicochetSFX(
        hmp::sfx::wave::sine, hmp::sfx::MID_A_FREQ * 4.0, hmp::sfx::VOLUME, 0.0 );
    const static hmp::sfx::waveform_t csScoreSFX(
        hmp::sfx::wave::triangle, hmp::sfx::MID_F_FREQ * 8.0, hmp::sfx::VOLUME, 0.0 );

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

    { // Entity Updates //
        pState->boundsEnt.update( pState->dt );
        for( uint8_t sideIdx = 0; sideIdx < 2; sideIdx++ )
            pState->ricochetEnts[sideIdx].update( pState->dt );
        pState->scoreEnt.update( pState->dt );
        pState->ballEnt.update( pState->dt );
        for( uint8_t sideIdx = 0; sideIdx < 2; sideIdx++ )
            pState->paddleEnts[sideIdx].update( pState->dt );
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

            pState->synth.play( &csWallRicochetSFX, hmp::sfx::BLIP_TIME );
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
                pState->pmid = hmp::mode::menu_id;
            }

            pState->synth.play( &csScoreSFX, hmp::sfx::BLIP_TIME );
        }
    }

    for( uint8_t paddleIdx = 0; paddleIdx < 2; paddleIdx++ ) {
        hmp::paddle_t& paddleEnt = pState->paddleEnts[paddleIdx];
        if( paddleEnt.mBBox.overlaps(ballEnt.mBBox) ) {
            ballEnt.ricochet( &paddleEnt );
            ballEnt.change( static_cast<hmp::team::team_e>(paddleEnt.mTeam) );
            ballEnt.mVel *= 1.1f;

            pState->synth.play( &csPaddleRicochetSFX, hmp::sfx::BLIP_TIME );
        } if( !boundsEnt.mBBox.contains(paddleEnt.mBBox) ) {
            paddleEnt.mBBox.embed( boundsEnt.mBBox );
        }
    }

    return true;
}


bool32_t game::render( const hmp::state_t* pState, const hmp::input_t* pInput, const hmp::output_t* pOutput ) {
    { // Render State //
        hmp::gfx::fbo_context_t simFBOC(
            pOutput->gfxBufferFBOs[hmp::GFX_BUFFER_SIM],
            pOutput->gfxBufferRess[hmp::GFX_BUFFER_SIM] );

        { // Entity Renders //
            pState->boundsEnt.render();
            for( uint8_t sideIdx = 0; sideIdx < 2; sideIdx++ )
                pState->ricochetEnts[sideIdx].render();
            pState->ballEnt.render();
            for( uint8_t sideIdx = 0; sideIdx < 2; sideIdx++ )
                pState->paddleEnts[sideIdx].render();
        }

        if( !pState->roundStarted ) {
            const hmp::ball_t& ball = pState->ballEnt;
            hmp::gfx::vector::render( ball.mBBox.center(), ball.mVel, 0.15f, ball.mColor );
        }
    }

    { // Render UI //
        hmp::gfx::fbo_context_t simFBOC(
            pOutput->gfxBufferFBOs[hmp::GFX_BUFFER_UI],
            pOutput->gfxBufferRess[hmp::GFX_BUFFER_UI] );

        pState->scoreEnt.render();
    }

    { // Render Master //
        const uint32_t masterFBO = pOutput->gfxBufferFBOs[hmp::GFX_BUFFER_MASTER];
        const vec2u32_t masterRes = pOutput->gfxBufferRess[hmp::GFX_BUFFER_MASTER];
        hmp::gfx::fbo_context_t masterFBOC( masterFBO, masterRes );

        hmp::gfx::render_context_t hmpRC( hmp::box_t(0.0f, 0.0f, 1.0f, 1.0f), &hmp::color::BACKGROUND );
        hmpRC.render();

        for( uint32_t gfxBufferIdx = 0; gfxBufferIdx < hmp::GFX_BUFFER_COUNT; gfxBufferIdx++ ) {
            const uint32_t gfxBufferFBO = pOutput->gfxBufferFBOs[gfxBufferIdx];
            const vec2u32_t& gfxBufferRes = pOutput->gfxBufferRess[gfxBufferIdx];
            const hmp::box_t& gfxBufferBox = pOutput->gfxBufferBoxs[gfxBufferIdx];

            glBindFramebuffer( GL_READ_FRAMEBUFFER, gfxBufferFBO );
            glBindFramebuffer( GL_DRAW_FRAMEBUFFER, masterFBO );
            for( uint32_t bufferTypeIdx = 0; bufferTypeIdx < 2; bufferTypeIdx++ ) {
                glBlitFramebuffer( 0, 0, gfxBufferRes.x, gfxBufferRes.y,
                    gfxBufferBox.min().x * masterRes.x, gfxBufferBox.min().y * masterRes.y,
                    gfxBufferBox.max().x * masterRes.x, gfxBufferBox.max().y * masterRes.y,
                    bufferTypeIdx ? GL_COLOR_BUFFER_BIT : GL_DEPTH_BUFFER_BIT,
                    bufferTypeIdx ? GL_LINEAR : GL_NEAREST );
            }
        }
    }

    return true;
}

/// 'hmp::mode::menu' Functions  ///

bool32_t menu::init( hmp::state_t* pState ) {
    pState->menuIdx = 0;

    return true;
}


bool32_t menu::update( hmp::state_t* pState, hmp::input_t* pInput, const float64_t pDT ) {
    const static hmp::sfx::waveform_t csMenuChangeSFX(
        hmp::sfx::wave::sawtooth, hmp::sfx::MID_C_FREQ, hmp::sfx::VOLUME, 0.0 );
    const static hmp::sfx::waveform_t csMenuSelectSFX(
        hmp::sfx::wave::sawtooth, hmp::sfx::MID_C_FREQ * 2.0, hmp::sfx::VOLUME, 0.0 );

    int32_t dy[2] = { 0, 0 };
    bool32_t dselect = false;

    if( llce::input::isKeyPressed(pInput->keyboard, SDL_SCANCODE_D) ) {
        dselect = true;
    } if( llce::input::isKeyPressed(pInput->keyboard, SDL_SCANCODE_L) ) {
        dselect = true;
    }

    if( llce::input::isKeyPressed(pInput->keyboard, SDL_SCANCODE_W) ) {
        dy[0] += 1;
    } if( llce::input::isKeyPressed(pInput->keyboard, SDL_SCANCODE_S) ) {
        dy[0] -= 1;
    } if( llce::input::isKeyPressed(pInput->keyboard, SDL_SCANCODE_I) ) {
        dy[1] += 1;
    } if( llce::input::isKeyPressed(pInput->keyboard, SDL_SCANCODE_K) ) {
        dy[1] -= 1;
    }

    if( dselect ) {
        pState->synth.play( &csMenuSelectSFX, hmp::sfx::BLIP_TIME );
        if( pState->menuIdx == 0 ) {
            pState->pmid = hmp::mode::game_id;
        } else if( pState->menuIdx == 1 ) {
            pState->pmid = hmp::mode::exit_id;
        }
    } else {
        if( dy[0] + dy[1] != 0 ) {
            pState->synth.play( &csMenuChangeSFX, hmp::sfx::BLIP_TIME );
        }
        pState->menuIdx = ( pState->menuIdx + dy[0] + dy[1] ) % hmp::MENU_ITEM_COUNT;
    }

    return true;
}

bool32_t menu::render( const hmp::state_t* pState, const hmp::input_t* pInput, const hmp::output_t* pOutput ) {
    const uint32_t masterFBO = pOutput->gfxBufferFBOs[hmp::GFX_BUFFER_MASTER];
    const vec2u32_t masterRes = pOutput->gfxBufferRess[hmp::GFX_BUFFER_MASTER];
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

    return true;
}

/// 'hmp::mode::pause' Functions  ///

bool32_t pause::init( hmp::state_t* pState ) {
    // TODO(JRC)
    return true;
}


bool32_t pause::update( hmp::state_t* pState, hmp::input_t* pInput, const float64_t pDT ) {
    // TODO(JRC)
    return true;
}


bool32_t pause::render( const hmp::state_t* pState, const hmp::input_t* pInput, const hmp::output_t* pOutput ) {
    // TODO(JRC)
    return true;
}

/// 'hmp::mode::reset' Functions  ///

bool32_t reset::init( hmp::state_t* pState ) {
    // TODO(JRC)
    return true;
}


bool32_t reset::update( hmp::state_t* pState, hmp::input_t* pInput, const float64_t pDT ) {
    // TODO(JRC)
    return true;
}


bool32_t reset::render( const hmp::state_t* pState, const hmp::input_t* pInput, const hmp::output_t* pOutput ) {
    // TODO(JRC)
    return true;
}

}

}
