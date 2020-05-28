#include <cstring>
#include <sstream>

#include <SDL2/SDL_opengl.h>

#include <glm/common.hpp>
#include <glm/geometric.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/ext/scalar_constants.hpp>

#include "gfx.h"
#include "geom.h"
#include "hmp_entities.h"

namespace hmp {

/// 'hmp::team_entity_t' Functions ///

team_entity_t::team_entity_t( const llce::box_t& pBBox, const team::team_e& pTeam ) :
        entity_t( pBBox, &hmp::color::TEAM[pTeam] ), mTeam( pTeam ) {
    
}


void team_entity_t::change( const team::team_e& pTeam ) {
    mColor = &hmp::color::TEAM[(mTeam = pTeam)];
}

/// 'hmp::bounds_t' Functions ///

bounds_t::bounds_t( const llce::box_t& pBBox ) :
        entity_t( pBBox, &hmp::color::BACKGROUND ) {
    
}


void bounds_t::render() const {
    llce::gfx::render_context_t baseRC( mBBox );
    llce::gfx::color_context_t baseCC( mColor );
    llce::gfx::render::box();

    // TODO(JRC): This 'brighten' operation only works because the source color
    // for the 'bounds_t' object is such that values don't overflow. In the future,
    // we need a more proper 'brighten' function that puts a ceiling on color values.
    const color4u8_t lineColor = static_cast<uint8_t>( 2 ) * *mColor;
    const llce::box_t lineBox( 0.5f, 0.5f, bounds_t::LINE_WIDTH, 1.0f, llce::geom::anchor2D::mm );
    llce::gfx::color_context_t lineCC( &lineColor );
    llce::gfx::render::box( lineBox );
}

/// 'hmp::ball_t' Functions ///

ball_t::ball_t( const llce::box_t& pBBox ) :
        team_entity_t( pBBox, hmp::team::neutral ) {
    
}


void ball_t::ricochet( const entity_t* pSurface ) {
    vec2f32_t contactVec = mBBox.mid() - pSurface->mBBox.mid();
    vec2f32_t contactNormal = contactVec; {
        llce::interval_t ballBoundsX = mBBox.xbounds(), ballBoundsY = mBBox.ybounds();
        llce::interval_t surfBoundsX = pSurface->mBBox.xbounds(), surfBoundsY = pSurface->mBBox.ybounds();

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

    vec2f32_t refspaceNormal = ( glm::length(pSurface->mVel) > glm::epsilon<float32_t>() ) ?
        glm::normalize( pSurface->mVel ) : vec2f32_t( 0.0f, 0.0f );

    vec2f32_t ricochetNormal = glm::normalize(
        0.5f * glm::normalize(glm::reflect(mVel, contactNormal)) +
        0.5f * refspaceNormal );
    float32_t ricochetAngle = glm::orientedAngle( contactNormal, ricochetNormal );

    vec2f32_t ricochetMinNormal, ricochetMaxNormal; {
        vec3f32_t vecRotate3d( contactNormal.x, contactNormal.y, 1.0f );
        vec3f32_t vecMinRotate3d =
            glm::rotate( mat3f32_t(1.0f), -ball_t::MAX_RICOCHET_ANGLE ) * vecRotate3d;
        vec3f32_t vecMaxRotate3d =
            glm::rotate( mat3f32_t(1.0f), +ball_t::MAX_RICOCHET_ANGLE ) * vecRotate3d;

        ricochetMinNormal = vec2f32_t( vecMinRotate3d.x, vecMinRotate3d.y );
        ricochetMaxNormal = vec2f32_t( vecMaxRotate3d.x, vecMaxRotate3d.y );
    }

    ricochetNormal = (
        (ricochetAngle < -ball_t::MAX_RICOCHET_ANGLE) ? ricochetMinNormal : (
        (ricochetAngle > +ball_t::MAX_RICOCHET_ANGLE) ? ricochetMaxNormal : (
        ricochetNormal )));
    mVel = glm::length( mVel ) * ricochetNormal;
    mBBox.exbed( pSurface->mBBox );
}

/// 'hmp::paddle_t' Functions ///

paddle_t::paddle_t( const llce::box_t& pBBox, const team::team_e& pTeam ) :
        team_entity_t( pBBox, pTeam ), mDX( 0 ), mDY( 0 ) {
    
}


void paddle_t::move( const int32_t pDX, const int32_t pDY ) {
    mDX = glm::clamp( pDX, -1, 1 );
    mDY = glm::clamp( pDY, -1, 1 );
}


void paddle_t::update( const float64_t pDT ) {
    mVel.x = mDX * paddle_t::MOVE_VEL;
    mVel.y = mDY * paddle_t::MOVE_VEL;

    mLifetime += pDT;
    mBBox.mPos += static_cast<float32_t>( pDT ) * mVel;
}

/// 'hmp::scoreboard_t' Functions ///

scoreboard_t::scoreboard_t( const llce::box_t& pBBox ) :
        entity_t( pBBox, &hmp::color::BORDER ) {
    mScores[hmp::team::west] = hmp::WINNING_SCORE;
    mScores[hmp::team::east] = hmp::WINNING_SCORE;
}


void scoreboard_t::tally( const int8_t pWestDelta, const int8_t pEastDelta ) {
    mScores[hmp::team::west] = std::max( 0, mScores[hmp::team::west] + pWestDelta );
    mScores[hmp::team::east] = std::max( 0, mScores[hmp::team::east] + pEastDelta );
}


void scoreboard_t::render() const {
    llce::gfx::render_context_t entityRC( mBBox );
    llce::gfx::color_context_t entityCC( mColor );

    for( int8_t team = hmp::team::west; team <= hmp::team::east; team++ ) {
        const int8_t teamScore = mScores[team];
        const float32_t teamScoreScaled = teamScore / ( hmp::WINNING_SCORE + 0.0f );
        const color4u8_t* teamColor = &hmp::color::TEAM[team];
        const color4u8_t teamColorScaled = 0.5f * static_cast<color4f32_t>( *teamColor );
        const bool8_t isTeamWest = team == hmp::team::west;

        const auto teamAnchor = isTeamWest ? llce::geom::anchor2D::hl : llce::geom::anchor2D::ll;
        const llce::box_t teamBox( 0.5f, 0.0f, 0.5f, 1.0f, teamAnchor ); {
            llce::gfx::render_context_t teamRC( teamBox );
            entityCC.update( &hmp::color::INTERFACE );
            llce::gfx::render::box();

            entityCC.update( &teamColorScaled );
            llce::gfx::render::box( llce::box_t(
                isTeamWest, 0.0f, teamScoreScaled, 1.0f, teamAnchor) );

            const float32_t teamScoreSize = 1.0f - 4.0f * scoreboard_t::BORDER_SIZE;
            char teamScoreBuffer[2];
            std::snprintf( &teamScoreBuffer[0], sizeof(teamScoreBuffer), "%d", teamScore );
            entityCC.update( teamColor );
            llce::gfx::render::text( &teamScoreBuffer[0], llce::box_t(
                0.5f, 0.5f, teamScoreSize, teamScoreSize,
                llce::geom::anchor2D::mm) );

            entityCC.update( &hmp::color::BORDER );
            llce::gfx::render::border( scoreboard_t::BORDER_SIZE, scoreboard_t::BORDER_DIM );
        }
    }
}

}
