#include <cstring>
#include <sstream>

#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_opengl_glext.h>

#include <glm/common.hpp>
#include <glm/geometric.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/ext/scalar_constants.hpp>

#include "gui.h"
#include "gfx.h"
#include "sfx.h"
#include "geom.h"
#include "util.hpp"

#include "hmp_modes.h"
#include "hmp_data.h"
#include "hmp_entities.h"

namespace hmp {

namespace mode {

/// Helper Structures ///

constexpr static uint32_t TEAM_UP_ACTIONS[] = { hmp::action::lup, hmp::action::rup, hmp::action::unbound };
constexpr static uint32_t TEAM_DOWN_ACTIONS[] = { hmp::action::ldn, hmp::action::rdn, hmp::action::unbound };
constexpr static uint32_t TEAM_LEFT_ACTIONS[] = { hmp::action::llt, hmp::action::rlt, hmp::action::unbound };
constexpr static uint32_t TEAM_RIGHT_ACTIONS[] = { hmp::action::lrt, hmp::action::rrt, hmp::action::unbound };
constexpr static uint32_t PAUSE_ACTION = hmp::action::etc;

constexpr static uint32_t MENU_ACTIONS[] = { hmp::action::unbound, hmp::action::lrt, hmp::action::ldn, hmp::action::lup };

constexpr static char8_t TITLE_ITEM_TEXT[][32] = { "START", "EXIT " };
constexpr static uint32_t TITLE_ITEM_COUNT = LLCE_ELEM_COUNT( TITLE_ITEM_TEXT );
constexpr static char8_t RESET_ITEM_TEXT[][32] = { "REPLAY", "EXIT  " };
constexpr static uint32_t RESET_ITEM_COUNT = LLCE_ELEM_COUNT( RESET_ITEM_TEXT );

const static auto SFX_MENU_CHANGE = llce::sfx::waveform::sawtooth<'c', 0, 4>;
const static auto SFX_MENU_SELECT = llce::sfx::waveform::sawtooth<'c', 0, 5>;

/// Helper Functions ///

void render_gameboard( const hmp::state_t* pState, const hmp::input_t* pInput, const hmp::output_t* pOutput ) {
    llce::gfx::fbo_context_t simFBOC(
        pOutput->gfxBufferFBOs[hmp::GFX_BUFFER_SIM_ID],
        pOutput->gfxBufferRess[hmp::GFX_BUFFER_SIM_ID] );

    if( pState->roundPaused ) {
        llce::gfx::color_context_t boardCC( &hmp::color::BACKGROUND );
        llce::gfx::render::box();

        llce::gfx::color_context_t textCC( &hmp::color::BACKGROUND2 );
        llce::gfx::render::text( "PAUSE",
            llce::box_t(0.5f, 0.5f, 1.0f, 0.25f, llce::geom::anchor2D::mm) );
    } else {
        pState->boundsEnt.render();
        for( uint8_t sideIdx = 0; sideIdx < 2; sideIdx++ ) {
            pState->ricochetEnts[sideIdx].render();
        }
        pState->ballEnt.render();
        for( uint8_t sideIdx = 0; sideIdx < 2; sideIdx++ ) {
            pState->paddleEnts[sideIdx].render();
        }

        if( !pState->roundStarted ) {
            const hmp::ball_t& ball = pState->ballEnt;
            llce::gfx::color_context_t ballCC( ball.mColor );
            llce::gfx::render::vector( ball.mBBox.mid(), ball.mVel, 0.15f );
        }
    }
}


void render_scoreboard( const hmp::state_t* pState, const hmp::input_t* pInput, const hmp::output_t* pOutput ) {
    llce::gfx::fbo_context_t simFBOC(
        pOutput->gfxBufferFBOs[hmp::GFX_BUFFER_UI_ID],
        pOutput->gfxBufferRess[hmp::GFX_BUFFER_UI_ID] );

    pState->scoreEnt.render();
}


void render_rasterize( const hmp::state_t* pState, const hmp::input_t* pInput, const hmp::output_t* pOutput ) {
    const uint32_t masterFBO = pOutput->gfxBufferFBOs[hmp::GFX_BUFFER_MASTER_ID];
    const vec2u32_t masterRes = pOutput->gfxBufferRess[hmp::GFX_BUFFER_MASTER_ID];
    llce::gfx::fbo_context_t masterFBOC( masterFBO, masterRes );

    llce::gfx::color_context_t hmpCC( &hmp::color::BACKGROUND );
    llce::gfx::render::box();

    for( uint32_t gfxBufferIdx = 0; gfxBufferIdx < hmp::GFX_BUFFER_COUNT; gfxBufferIdx++ ) {
        const uint32_t gfxBufferFBO = pOutput->gfxBufferFBOs[gfxBufferIdx];
        const vec2u32_t& gfxBufferRes = pOutput->gfxBufferRess[gfxBufferIdx];
        const llce::box_t& gfxBufferBox = pOutput->gfxBufferBoxs[gfxBufferIdx];

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

/// 'hmp::mode::game' Functions  ///

bool32_t game::init( hmp::state_t* pState, hmp::input_t* pInput ) {
    pState->rt = 0.0;
    pState->roundStarted = false;
    pState->roundPaused = false;
    pState->roundServer = hmp::team::east;

    const vec2f32_t boundsBasePos( 0.0f, 0.0f ), boundsDims( 1.0f, 1.0f );
    pState->boundsEnt = hmp::bounds_t( llce::box_t(boundsBasePos, boundsDims) );

    for( uint32_t ricochetIdx = 0; ricochetIdx < 2; ricochetIdx++ ) {
        const vec2f32_t boundsPos = boundsBasePos + vec2f32_t( 0.0f, (ricochetIdx != 0) ? 1.0f : -1.0f );
        pState->ricochetEnts[ricochetIdx] = hmp::bounds_t( llce::box_t(boundsPos, boundsDims) );
    }

    const vec2f32_t scoreBasePos( 0.0f, 0.0f ), scoreDims( 1.0f, 1.0f );
    pState->scoreEnt = hmp::scoreboard_t( llce::box_t(scoreBasePos, scoreDims) );

    const vec2f32_t ballDims( 2.5e-2f, 2.5e-2f );
    const vec2f32_t ballPos = vec2f32_t( 0.5f, 0.5f ) - 0.5f * ballDims;
    pState->ballEnt = hmp::ball_t( llce::box_t(ballPos, ballDims) );

    const vec2f32_t paddleDims( 2.5e-2f, 1.0e-1f );
    const vec2f32_t westPos = vec2f32_t( 2.0f * paddleDims[0], 0.5f - 0.5f * paddleDims[1] );
    pState->paddleEnts[hmp::team::west] = hmp::paddle_t( llce::box_t(westPos, paddleDims), hmp::team::west );
    const vec2f32_t eastPos = vec2f32_t( 1.0f - 3.0f * paddleDims[0], 0.5f - 0.5f * paddleDims[1] );
    pState->paddleEnts[hmp::team::east] = hmp::paddle_t( llce::box_t(eastPos, paddleDims), hmp::team::east );

    return true;
}


bool32_t game::update( hmp::state_t* pState, hmp::input_t* pInput, const float64_t pDT ) {
    const static auto csPaddleRicochetSFX = llce::sfx::waveform::sine<'c', 1, 6>;
    const static auto csWallRicochetSFX = llce::sfx::waveform::sine<'a', 0, 5>;
    const static auto csScoreSFX = llce::sfx::waveform::triangle<'f', 0, 6>;

    // Process Inputs //

    int32_t dx[2] = { 0, 0 }, dy[2] = { 0, 0 };

    if( pInput->isPressedAct(PAUSE_ACTION) ) {
        pState->roundPaused = !pState->roundPaused;
    }

    for( uint8_t team = hmp::team::west; team <= hmp::team::east; team++ ) {
        if( pInput->isDownAct(TEAM_UP_ACTIONS[team]) ) {
            dy[team] += 1;
        } if( pInput->isDownAct(TEAM_DOWN_ACTIONS[team]) ) {
            dy[team] -= 1;
        } if( pInput->isDownAct(TEAM_LEFT_ACTIONS[team]) ) {
            dx[team] -= 1;
        } if( pInput->isDownAct(TEAM_RIGHT_ACTIONS[team]) ) {
            dx[team] += 1;
        }
    }

    // Update State //

    if( pState->roundPaused ) { return true; }

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
        float64_t ballThetaSeed = pState->rng.nextf();
        float64_t ballTheta = 2 * ballMaxTheta * ballThetaSeed - ballMaxTheta;
        pState->ballEnt.mBBox.mPos =
            vec2f32_t( 0.5f, 0.5f ) - 0.5f * pState->ballEnt.mBBox.mDims;
        pState->ballEnt.mVel = hmp::ball_t::HINT_VEL *
            vec2f32_t( roundDir * glm::cos(ballTheta), glm::sin(ballTheta) );
        pState->ballEnt.change( hmp::team::team_e::neutral );
    } if( !pState->roundStarted && pState->rt >= hmp::ROUND_START_TIME ) {
        pState->ballEnt.mVel *= hmp::ball_t::MOVE_VEL / glm::length( pState->ballEnt.mVel );
        pState->roundStarted = true;
    }

    // Resolve Collisions //

    hmp::bounds_t& boundsEnt = pState->boundsEnt;
    hmp::ball_t& ballEnt = pState->ballEnt;

    if( !boundsEnt.mBBox.contains(ballEnt.mBBox) ) {
        llce::interval_t ballX = ballEnt.mBBox.xbounds(), ballY = ballEnt.mBBox.ybounds();
        llce::interval_t boundsX = boundsEnt.mBBox.xbounds(), boundsY = boundsEnt.mBBox.ybounds();
        if( !boundsY.contains(ballY) ) {
            uint8_t ricochetIdx = (uint8_t)( ballY.mMax > boundsY.mMax );
            ballEnt.ricochet( &pState->ricochetEnts[ricochetIdx] );

            pState->synth.play( csWallRicochetSFX, hmp::sfx::BLIP_TIME );
        } if( !boundsX.contains(ballX) ) {
            bool8_t isWestScore = ballX.contains( boundsX.mMax );
            pState->scoreEnt.tally( isWestScore ? -1 : 0, isWestScore ? 0 : -1 );
            pState->roundServer = isWestScore ? hmp::team::east : hmp::team::west;

            // NOTE(JRC): Triggers the simulation loop to set the new speed and
            // location for the ball.
            ballEnt.mVel = vec2f32_t( 0.0f, 0.0f );

            pState->rt = 0.0f;
            pState->roundStarted = false;

            if( pState->scoreEnt.mScores[0] <= 0 || pState->scoreEnt.mScores[1] <= 0 ) {
                pState->pmid = hmp::mode::reset_id;
            }

            pState->synth.play( csScoreSFX, hmp::sfx::BLIP_TIME );
        }
    }

    for( uint8_t paddleIdx = 0; paddleIdx < 2; paddleIdx++ ) {
        hmp::paddle_t& paddleEnt = pState->paddleEnts[paddleIdx];
        if( paddleEnt.mBBox.overlaps(ballEnt.mBBox) ) {
            ballEnt.ricochet( &paddleEnt );
            ballEnt.change( static_cast<hmp::team::team_e>(paddleEnt.mTeam) );
            ballEnt.mVel *= 1.1f;

            pState->synth.play( csPaddleRicochetSFX, hmp::sfx::BLIP_TIME );
        } if( !boundsEnt.mBBox.contains(paddleEnt.mBBox) ) {
            paddleEnt.mBBox.embed( boundsEnt.mBBox );
        }
    }

    return true;
}


bool32_t game::render( const hmp::state_t* pState, const hmp::input_t* pInput, const hmp::output_t* pOutput ) {
    render_gameboard( pState, pInput, pOutput );
    render_scoreboard( pState, pInput, pOutput );
    render_rasterize( pState, pInput, pOutput );

    return true;
}

/// 'hmp::mode::menu' Functions  ///

bool32_t title::init( hmp::state_t* pState, hmp::input_t* pInput ) {
    auto cTitleItems = llce::util::pointerize( TITLE_ITEM_TEXT );
    pState->titleMenu = llce::gui::menu_t(
        pInput, &MENU_ACTIONS[0],
        "HMP", cTitleItems.data(), TITLE_ITEM_COUNT,
        &hmp::color::BACKGROUND,
        &hmp::color::BACKGROUND2,
        &hmp::color::TEAM[hmp::team::neutral] );

    return true;
}


bool32_t title::update( hmp::state_t* pState, hmp::input_t* pInput, const float64_t pDT ) {
    pState->titleMenu.update( pDT );

    if( pState->titleMenu.changed(llce::gui::event::select) ) {
        pState->synth.play( SFX_MENU_SELECT, hmp::sfx::BLIP_TIME );
        if( pState->titleMenu.mItemIndex == 0 ) {
            pState->pmid = hmp::mode::game_id;
        } else if( pState->titleMenu.mItemIndex == 1 ) {
            pState->pmid = hmp::mode::exit_id;
        }
    } else if( pState->titleMenu.changed(llce::gui::event::next) ||
            pState->titleMenu.changed(llce::gui::event::prev) ) {
        pState->synth.play( SFX_MENU_CHANGE, hmp::sfx::BLIP_TIME );
    }

    return true;
}

bool32_t title::render( const hmp::state_t* pState, const hmp::input_t* pInput, const hmp::output_t* pOutput ) {
    const uint32_t masterFBO = pOutput->gfxBufferFBOs[hmp::GFX_BUFFER_MASTER_ID];
    const vec2u32_t masterRes = pOutput->gfxBufferRess[hmp::GFX_BUFFER_MASTER_ID];
    llce::gfx::fbo_context_t masterFBOC( masterFBO, masterRes );

    pState->titleMenu.render();

    return true;
}

/// 'hmp::mode::reset' Functions  ///

bool32_t reset::init( hmp::state_t* pState, hmp::input_t* pInput ) {
    auto cResetItems = llce::util::pointerize( RESET_ITEM_TEXT );
    pState->resetMenu = llce::gui::menu_t(
        pInput, &MENU_ACTIONS[0],
        "", cResetItems.data(), RESET_ITEM_COUNT,
        &hmp::color::BACKGROUND,
        &hmp::color::BACKGROUND2,
        &hmp::color::TEAM[hmp::team::neutral] );
    pState->resetMenuUpdated = false;

    return true;
}


bool32_t reset::update( hmp::state_t* pState, hmp::input_t* pInput, const float64_t pDT ) {
    pState->resetMenu.update( pDT );

    if( pState->resetMenu.changed(llce::gui::event::select) ) {
        pState->synth.play( SFX_MENU_SELECT, hmp::sfx::BLIP_TIME );
        if( pState->resetMenu.mItemIndex == 0 ) {
            pState->pmid = hmp::mode::game_id;
        } else if( pState->resetMenu.mItemIndex == 1 ) {
            pState->pmid = hmp::mode::title_id;
        }
    } else if( pState->resetMenu.changed(llce::gui::event::next) ||
            pState->resetMenu.changed(llce::gui::event::prev) ) {
        pState->synth.play( SFX_MENU_CHANGE, hmp::sfx::BLIP_TIME );
    }

    if( !pState->resetMenuUpdated ) { // Set Render Header Based on Winner //
        const char8_t cTeamNames[2][8] = { "WEST", "EAST" };
        const auto cTeamWinner = ( pState->scoreEnt.mScores[hmp::team::west] <= 0 ) ?
            hmp::team::west : hmp::team::east;

        char8_t headerText[16];
        std::snprintf( &headerText[0], sizeof(headerText),
            "%s WINS!", &cTeamNames[cTeamWinner][0] );
        const color4u8_t* headerColor = &hmp::color::TEAM[cTeamWinner];

        std::strcpy( &pState->resetMenu.mTitle[0], &headerText[0] );
        pState->resetMenu.mColorText = headerColor;
        pState->resetMenuUpdated = true;
    }

    return true;
}


bool32_t reset::render( const hmp::state_t* pState, const hmp::input_t* pInput, const hmp::output_t* pOutput ) {
    { // Render Reset Menu //
        llce::gfx::fbo_context_t menuFBOC(
            pOutput->gfxBufferFBOs[hmp::GFX_BUFFER_SIM_ID],
            pOutput->gfxBufferRess[hmp::GFX_BUFFER_SIM_ID] );

        pState->resetMenu.render();
    }

    render_scoreboard( pState, pInput, pOutput );
    render_rasterize( pState, pInput, pOutput );

    return true;
}

}

}
