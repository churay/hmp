#include <cstring>
#include <sstream>

#include <SDL2/SDL_opengl.h>

#include <glm/common.hpp>
#include <glm/geometric.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/ext/vector_float2.hpp>

#include "hmp_gfx.h"
#include "hmp_entities.h"

namespace hmp {

/// 'hmp::team_entity_t' Functions ///

team_entity_t::team_entity_t( const box_t& pBBox, const team::team_e& pTeam ) :
        entity_t( pBBox, &hmp::color::TEAM[pTeam] ), mTeam( pTeam ) {
    
}

/// 'hmp::bounds_t' Functions ///

bounds_t::bounds_t( const box_t& pBBox ) :
        entity_t( pBBox, &hmp::color::BACKGROUND ) {
    
}


void bounds_t::irender() const {
    entity_t::irender();

    // TODO(JRC): This 'brighten' operation only works because the source color
    // for the 'bounds_t' object is such that values don't overflow. In the future,
    // we need a more proper 'brighten' function that puts a ceiling on color values.
    const color32_t entityColor = static_cast<uint8_t>( 2 ) * *mColor;
    hmp::gfx::render_context_t entityRC(
        box_t(0.5f, 0.5f, bounds_t::LINE_WIDTH, 1.0f, box_t::anchor_e::c), &entityColor );
    entityRC.render();
}

/// 'hmp::ball_t' Functions ///

ball_t::ball_t( const box_t& pBBox ) :
        team_entity_t( pBBox, hmp::team::neutral ) {
    
}


void ball_t::ricochet( const entity_t* pSurface ) {
    glm::vec2 contactVec = mBBox.center() - pSurface->mBBox.center();
    glm::vec2 contactNormal = contactVec; {
        interval_t ballBoundsX = mBBox.xbounds(), ballBoundsY = mBBox.ybounds();
        interval_t surfBoundsX = pSurface->mBBox.xbounds(), surfBoundsY = pSurface->mBBox.ybounds();

        // TODO(JRC): If it becomes relevant, fix the case where the ball
        // is completely contained in the surface; these cases will need
        // some form of unique logic to handle.
        bool32_t surfContainsX = surfBoundsX.contains( ballBoundsX );
        bool32_t surfContainsY = surfBoundsY.contains( ballBoundsY );
        contactNormal.x *= ( !surfContainsX + 0.0f );
        contactNormal.y *= ( !surfContainsY + 0.0f );

        // NOTE(JRC): If the ball hits the corner of a surface, choose the
        // surface of greatest intersection for the contact normal.
        if( !surfContainsX && !surfContainsY ) {
            bool32_t biggerIntxX =
                surfBoundsX.intersect( ballBoundsX ).length() <
                surfBoundsY.intersect( ballBoundsY ).length();
            contactNormal.x *= ( biggerIntxX + 0.0f );
            contactNormal.y *= ( !biggerIntxX + 0.0f );
        }

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
    mBBox.exbed( pSurface->mBBox );
}

/// 'hmp::paddle_t' Functions ///

paddle_t::paddle_t( const box_t& pBBox, const team::team_e& pTeam ) :
        team_entity_t( pBBox, pTeam ), mDX( 0 ), mDY( 0 ) {
    
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
        entity_t( pBBox, &hmp::color::BORDER ) {
    mScores[hmp::team::west] = hmp::WINNING_SCORE;
    mScores[hmp::team::east] = hmp::WINNING_SCORE;
}


void scoreboard_t::tally( const int8_t pWestDelta, const int8_t pEastDelta ) {
    mScores[hmp::team::west] = std::max( 0, mScores[hmp::team::west] + pWestDelta );
    mScores[hmp::team::east] = std::max( 0, mScores[hmp::team::east] + pEastDelta );
}


void scoreboard_t::irender() const {
    const static bit8_t DIGIT_DISPLAY_LINES[10][3][3] = {
        { {1, 1, 0}, {1, 0, 1}, {1, 1, 0} },   // 0
        { {0, 0, 0}, {0, 0, 0}, {1, 1, 0} },   // 1
        { {1, 0, 0}, {1, 1, 1}, {0, 1, 0} },   // 2
        { {0, 0, 0}, {1, 1, 1}, {1, 1, 0} },   // 3
        { {0, 1, 0}, {0, 1, 0}, {1, 1, 0} },   // 4
        { {0, 1, 0}, {1, 1, 1}, {1, 0, 0} },   // 5
        { {1, 1, 0}, {1, 1, 1}, {1, 0, 0} },   // 6
        { {0, 0, 0}, {0, 0, 1}, {1, 1, 0} },   // 7
        { {1, 1, 0}, {1, 1, 1}, {1, 1, 0} },   // 8
        { {0, 1, 0}, {1, 1, 1}, {1, 1, 0} }    // 9
    };

    const static float32_t DIGIT_LINE_WIDTH =
        ( (1.0f - 4.0f * scoreboard_t::TALLY_RADIUS) / 1.0f );
    const static float32_t DIGIT_LINE_HEIGHT =
        ( (1.0f - 6.0f * scoreboard_t::TALLY_RADIUS) / 2.0f );
    const static float32_t DIGIT_LINE_OFFSET =
        ( 2.0f * scoreboard_t::TALLY_RADIUS + DIGIT_LINE_HEIGHT );

    entity_t::irender();

    for( int8_t team = hmp::team::west; team <= hmp::team::east; team++ ) {
        const int8_t teamScore = mScores[team];
        const color32_t* teamColor = &hmp::color::TEAM[team];
        const bool8_t isTeamWest = team == hmp::team::west;

        const float32_t teamOrient = isTeamWest ? -1.0f : 1.0f;
        const auto teamAnchor = isTeamWest ? box_t::anchor_e::se : box_t::anchor_e::sw;
        const glm::vec2 teamBasePos = glm::vec2( 0.5f, scoreboard_t::PADDING_WIDTH ) +
            teamOrient * glm::vec2( scoreboard_t::PADDING_WIDTH, 0.0f );
        const glm::vec2 teamBaseDims = glm::vec2( 0.5f, 1.0f ) -
            ( 2.0f * scoreboard_t::PADDING_WIDTH * glm::vec2(1.0f, 1.0f) );

        const hmp::box_t teamBox( teamBasePos, teamBaseDims, teamAnchor ); {
            hmp::gfx::render_context_t teamRC( teamBox, &hmp::color::INTERFACE );
            teamRC.render();

            // TODO(JRC): This segment was removed because it doesn't interact well
            // with a digit overlay, but it could be reintroduced at some point.
            //
            // const float32_t teamScoreFrac = teamScore / (hmp::WINNING_SCORE + 0.0f);
            // const glm::vec2 scoreBasePos = glm::vec2( isTeamWest ? 1.0f : 0.0f, 0.0f ) +
            //     teamOrient * glm::vec2( 1.0f - teamScoreFrac, 0.0f );
            // const glm::vec2 scoreBaseDims = glm::vec2( teamScoreFrac, 1.0f );
            // const hmp::box_t scoreBox( scoreBasePos, scoreBaseDims, teamAnchor ); {
            //     hmp::gfx::render_context_t scoreRC( scoreBox, teamColor );
            //     scoreRC.render();
            // }

            // TODO(JRC): This code is a bit sloppy and should be improved if at
            // all possible.
            const auto digitLines = DIGIT_DISPLAY_LINES[teamScore];
            for( uint8_t colIdx = 0, lineIdx = 0; colIdx < 3; colIdx++ ) {
                for( uint8_t rowIdx = 0; rowIdx < 3; rowIdx++, lineIdx++ ) {
                    const bool8_t isLineVertical = colIdx % 2 == 0;

                    const float32_t lineX = scoreboard_t::TALLY_RADIUS + colIdx * (scoreboard_t::TALLY_RADIUS + DIGIT_LINE_WIDTH / 2.0f);
                    const float32_t lineY = isLineVertical ?
                        2.0f * scoreboard_t::TALLY_RADIUS + DIGIT_LINE_HEIGHT / 2.0f + rowIdx * DIGIT_LINE_OFFSET:
                        scoreboard_t::TALLY_RADIUS + rowIdx * DIGIT_LINE_OFFSET;
                    const float32_t lineWidth = !isLineVertical ? DIGIT_LINE_WIDTH : scoreboard_t::TALLY_RADIUS;
                    const float32_t lineHeight = isLineVertical ? DIGIT_LINE_HEIGHT : scoreboard_t::TALLY_RADIUS;
                    if( digitLines[colIdx][rowIdx] ) {
                        const hmp::box_t lineBox( lineX, lineY, lineWidth, lineHeight, box_t::anchor_e::c );
                        hmp::gfx::render_context_t lineRC( lineBox, teamColor );
                        lineRC.render();
                    }
                }
            }
        }
    }
}

}
