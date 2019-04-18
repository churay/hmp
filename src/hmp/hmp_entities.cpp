#include <cstring>
#include <sstream>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#include <glm/common.hpp>
#include <glm/geometric.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/ext/vector_float2.hpp>

#include <SDL2/SDL_opengl.h>

#include "hmp_gfx.h"
#include "hmp_entities.h"

namespace hmp {

/// 'hmp::bounds_t' Functions ///

bounds_t::bounds_t( const box_t& pBBox ) :
        entity_t( pBBox, hmp::BACKGROUND_COLOR ) {
    
}


void bounds_t::irender() const {
    entity_t::irender();

    hmp::gfx::render_context_t entityRC(
        box_t(0.5f, 0.5f, bounds_t::LINE_WIDTH, 1.0f, box_t::pos_type::c),
        llce::util::brighten(*mColor, 1.5f) );
    entityRC.render();
}

/// 'hmp::ball_t' Functions ///

ball_t::ball_t( const box_t& pBBox ) :
        entity_t( pBBox, hmp::TEAM_COLORS[static_cast<int>(hmp::team_e::neutral)] ), mTeam( hmp::team_e::neutral ) {
    
}


void ball_t::ricochet( const entity_t* pSurface ) {
    mBBox.exbed( pSurface->mBBox );

    glm::vec2 contactVec = mBBox.center() - pSurface->mBBox.center();
    glm::vec2 contactNormal = contactVec; {
        // TODO(JRC): If it becomes relevant, fix the case where the ball
        // is completely contained in the surface; these cases will need
        // some form of unique logic to handle.
        bool32_t surfContainsX = pSurface->mBBox.xbounds().contains( mBBox.xbounds() );
        bool32_t surfContainsY = pSurface->mBBox.ybounds().contains( mBBox.ybounds() );
        contactNormal.x *= ( !surfContainsX + 0.0f );
        contactNormal.y *= ( !surfContainsY + 0.0f );
        contactNormal = glm::normalize( contactNormal );
    }

    glm::vec2 refspaceNormal = ( glm::length(pSurface->mVel) > glm::epsilon<float32_t>() ) ?
        glm::normalize( pSurface->mVel ) : glm::vec2( 0.0f, 0.0f );

    glm::vec2 ricochetNormal = glm::normalize(
        0.5f * glm::normalize(glm::reflect(mVel, contactNormal)) +
        0.5f * refspaceNormal );
    float32_t ricochetAngle = glm::orientedAngle( contactNormal, ricochetNormal );

    glm::vec2 ricochetMinNormal, ricochetMaxNormal; {
        glm::vec3 vecRotate3d( contactNormal.x, contactNormal.y, 1.0f );
        glm::vec3 vecMinRotate3d =
            glm::rotate( glm::mat3(1.0f), -ball_t::MAX_RICOCHET_ANGLE ) * vecRotate3d;
        glm::vec3 vecMaxRotate3d =
            glm::rotate( glm::mat3(1.0f), +ball_t::MAX_RICOCHET_ANGLE ) * vecRotate3d;

        ricochetMinNormal = glm::vec2( vecMinRotate3d.x, vecMinRotate3d.y );
        ricochetMaxNormal = glm::vec2( vecMaxRotate3d.x, vecMaxRotate3d.y );
    }

    ricochetNormal = (
        (ricochetAngle < -ball_t::MAX_RICOCHET_ANGLE) ? ricochetMinNormal : (
        (ricochetAngle > +ball_t::MAX_RICOCHET_ANGLE) ? ricochetMaxNormal : (
        ricochetNormal )));
    mVel = glm::length( mVel ) * ricochetNormal;
}

/// 'hmp::paddle_t' Functions ///

paddle_t::paddle_t( const box_t& pBBox, const team_e& pTeam ) :
        entity_t( pBBox, hmp::TEAM_COLORS[static_cast<int>(pTeam)] ), mTeam( pTeam ), mDX( 0 ), mDY( 0 ) {
    
}


void paddle_t::move( const int32_t pDX, const int32_t pDY ) {
    mDX = glm::clamp( pDX, -1, 1 );
    mDY = glm::clamp( pDY, -1, 1 );
}


void paddle_t::iupdate( const float64_t pDT ) {
    mVel.x = mDX * paddle_t::MOVE_VEL;
    mVel.y = mDY * paddle_t::MOVE_VEL;
    entity_t::iupdate( pDT );
}

/// 'hmp::scoreboard_t' Functions ///

scoreboard_t::scoreboard_t( const box_t& pBBox ) :
        entity_t( pBBox, hmp::BORDER_COLOR ) {
    mScores[static_cast<int>(hmp::team_e::west)] = hmp::WINNING_SCORE;
    mScores[static_cast<int>(hmp::team_e::east)] = hmp::WINNING_SCORE;
}


void scoreboard_t::tally( const uint8_t pWestDelta, const uint8_t pEastDelta ) {
    mScores[static_cast<int>(hmp::team_e::west)] += pWestDelta;
    mScores[static_cast<int>(hmp::team_e::east)] += pEastDelta;
}


void scoreboard_t::irender() const {
    const static bit8_t DIGIT_DISPLAY_LINES[10][3][3] = {
        { {1, 1, 0}, {1, 0, 1}, {1, 1, 0} },   // 0
        { {0, 0, 0}, {0, 0, 0}, {1, 1, 0} },   // 1
        { {0, 1, 0}, {1, 1, 1}, {1, 0, 0} },   // 2
        { {0, 0, 0}, {1, 1, 1}, {1, 1, 0} },   // 3
        { {1, 0, 0}, {0, 1, 0}, {1, 1, 0} },   // 4
        { {1, 0, 0}, {1, 1, 1}, {0, 1, 0} },   // 5
        { {1, 1, 0}, {1, 1, 1}, {0, 1, 0} },   // 6
        { {0, 0, 0}, {1, 0, 0}, {1, 1, 0} },   // 7
        { {1, 1, 0}, {1, 1, 1}, {1, 1, 0} },   // 8
        { {1, 0, 0}, {1, 1, 1}, {1, 1, 0} }    // 9
    };

    entity_t::irender();

    for( uint8_t teamIdx = 0; teamIdx < 2; teamIdx++ ) {
        hmp::team_e team = static_cast<hmp::team_e>( teamIdx );

        const uint8_t teamScore = mScores[teamIdx];
        const color_t& teamColor = hmp::TEAM_COLORS[teamIdx];
        const auto teamAnchor = ( team == hmp::team_e::west ) ?
            box_t::pos_type::sw : box_t::pos_type::se;

        // TODO(JRC): Clean up this disgusting mess that you've made.
        hmp::box_t teamBox(
            glm::vec2((team == hmp::team_e::west) ? scoreboard_t::PADDING_WIDTH : 1.0f - scoreboard_t::PADDING_WIDTH, scoreboard_t::PADDING_WIDTH),
            glm::vec2(0.5f, 1.0f) - (2.0f * scoreboard_t::PADDING_WIDTH * glm::vec2(1.0f, 1.0f)),
            teamAnchor );
        hmp::gfx::render_context_t teamContext( teamBox, hmp::INTERFACE_COLOR );
        teamContext.render();

        color_t teamColorClear = teamColor;
        teamColorClear.a = 0xFF / 4;
        hmp::box_t scoreBox( 0.0f, 0.0f, teamScore / (hmp::WINNING_SCORE + 0.0f), 1.0f );
        hmp::gfx::render_context_t scoreContext( scoreBox, teamColorClear );
        scoreContext.render();

        // TODO(JRC): Digit w/ black outline but same color
    }
}

}
