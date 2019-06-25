#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_opengl_glext.h>

#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <cstring>

#include "hmp_gfx.h"

namespace hmp {

namespace gfx {

/// 'hmp::gfx::render_context_t' Functions ///

render_context_t::render_context_t( const box_t& pBox, const color4u8_t* pColor ) {
    glPushMatrix();
    glm::mat4 matModelWorld( 1.0f );
    matModelWorld *= glm::translate( glm::mat4(1.0f), glm::vec3(pBox.mPos.x, pBox.mPos.y, 0.0f) );
    matModelWorld *= glm::scale( glm::mat4(1.0f), glm::vec3(pBox.mDims.x, pBox.mDims.y, 1.0f) );
    glMultMatrixf( &matModelWorld[0][0] );

    glPushAttrib( GL_CURRENT_BIT );
    glColor4ubv( (uint8_t*)pColor );
}


// TODO(JRC): This is black magic from my 'fxn' project 'renderable_t'
// type that so happens to work... I need to reason this out again
// to figure out how this algorithm works.
render_context_t::render_context_t( const box_t& pBox, const float32_t pScreenRatio, const color4u8_t* pColor ) :
        render_context_t( pBox, pColor ) {
    // TODO(JRC): It would probably be a good idea to ultimately abstract out
    // this functionality into its own function (e.g. 'getOGLMatrix').
    glm::mat4 xformMatrix( 0.0f ); {
        glm::mat4 mvMatrix( 0.0f );
        glGetFloatv( GL_MODELVIEW_MATRIX, &mvMatrix[0][0] );

        glm::mat4 projMatrix( 0.0f );
        glGetFloatv( GL_PROJECTION_MATRIX, &projMatrix[0][0] );

        vec2u32_t vpCoords, vpRes; {
            int32_t viewParams[4];
            glGetIntegerv( GL_VIEWPORT, &viewParams[0] );
            vpCoords = { static_cast<uint32_t>(viewParams[0]), static_cast<uint32_t>(viewParams[1]) };
            vpRes = { static_cast<uint32_t>(viewParams[2]), static_cast<uint32_t>(viewParams[3]) };
        }

        glm::mat4 vpMatrix( 1.0f );
        vpMatrix = glm::translate( vpMatrix, glm::vec3((vpCoords.x + vpRes.x) / 2.0f, (vpCoords.y + vpRes.y) / 2.0f, 0.5f) );
        vpMatrix = glm::scale( vpMatrix, glm::vec3(vpRes.x / 2.0f, vpRes.y / 2.0f, 0.5f) );

        xformMatrix = vpMatrix * projMatrix * mvMatrix;
    }

    glm::vec4 currScreenDims = xformMatrix * glm::vec4( 1.0f, 1.0f, 0.0f, 0.0f );
    float32_t currScreenRatio = currScreenDims.x / currScreenDims.y;

    box_t ratioBox( 0.0f, 0.0f, 1.0f, 1.0f );
    float32_t wscaled = pScreenRatio * ratioBox.mDims.y / currScreenRatio;
    float32_t hscaled = currScreenRatio * ratioBox.mDims.x / pScreenRatio;
    if( wscaled < ratioBox.mDims.x ) {
        ratioBox.mPos.x += ( ratioBox.mDims.x - wscaled ) / 2.0f;
        ratioBox.mDims.x = wscaled;
    } else {
        ratioBox.mPos.y += ( ratioBox.mDims.y - hscaled ) / 2.0f;
        ratioBox.mDims.y = hscaled;
    }

    glm::mat4 matRatio( 1.0f );
    matRatio *= glm::translate( glm::mat4(1.0f), glm::vec3(ratioBox.mPos.x, ratioBox.mPos.y, 0.0f) );
    matRatio *= glm::scale( glm::mat4(1.0f), glm::vec3(ratioBox.mDims.x, ratioBox.mDims.y, 1.0f) );
    glMultMatrixf( &matRatio[0][0] );
}


render_context_t::~render_context_t() {
    glPopAttrib();
    glPopMatrix();
}


void render_context_t::render() const {
    glBegin( GL_QUADS ); {
        glVertex2f( 0.0f, 0.0f );
        glVertex2f( 1.0f, 0.0f );
        glVertex2f( 1.0f, 1.0f );
        glVertex2f( 0.0f, 1.0f );
    } glEnd();
}

/// 'hmp::gfx::fbo_context_t' Functions ///

fbo_context_t::fbo_context_t( const uint32_t pFBID, const vec2u32_t pFBRes ) {
    int32_t viewParams[4];
    glGetIntegerv( GL_VIEWPORT, &viewParams[0] );
    mViewCoords = { static_cast<uint32_t>(viewParams[0]), static_cast<uint32_t>(viewParams[1]) };
    mViewRes = { static_cast<uint32_t>(viewParams[2]), static_cast<uint32_t>(viewParams[3]) };

    glBindFramebuffer( GL_FRAMEBUFFER, pFBID );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glViewport( 0, 0, pFBRes.x, pFBRes.y );
}


fbo_context_t::~fbo_context_t() {
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    glViewport( mViewCoords.x, mViewCoords.y, mViewRes.x, mViewRes.y );
}

/// 'hmp::gfx::text' Functions ///

void text::render( const char8_t* pText, const color4u8_t* pColor ) {
    const static float64_t csDigitSpaceX = 1.0 / hmp::gfx::DIGIT_WIDTH;
    const static float64_t csDigitSpaceY = 1.0 / hmp::gfx::DIGIT_HEIGHT;
    const static float64_t csDigitPaddingX = csDigitSpaceX / 10.0;
    const static float64_t csDigitPaddingY = csDigitSpaceY / 10.0;
    const static float64_t csDigitFillX = csDigitSpaceX - 2.0 * csDigitPaddingX;
    const static float64_t csDigitFillY = csDigitSpaceY - 2.0 * csDigitPaddingY;

    const uint32_t cTextLength = std::strlen( pText );
    const float64_t cTextSpacingX = 2.0 * hmp::gfx::DIGIT_WIDTH;
    const float64_t cTextSpacingY = 0.0;
    const float64_t cTextFillX = cTextSpacingX / ( (cTextSpacingX + 1.0) * cTextLength - 1.0 );
    const float64_t cTextFillY = 1.0;

    const box_t cRenderBox( 0.0f, 0.0f, 1.0f, 1.0f );
    gfx::render_context_t rrc( cRenderBox, hmp::gfx::DIGIT_ASPECT * cTextLength, pColor );
    for( const char8_t* pTextItr = pText; *pTextItr != '\0'; pTextItr++ ) {
        const auto cTextDigitMap = &hmp::gfx::ASCII_DIGIT_MAP[static_cast<uint32_t>(*pTextItr)][0];
        const uint32_t cTextIdx = pTextItr - pText;

        const float64_t cTextOffsetX = ( cTextFillX + cTextFillX / cTextSpacingX ) * cTextIdx;
        const float64_t cTextOffsetY = 0.0;

        const box_t cTextBox( cTextOffsetX, cTextOffsetY, cTextFillX, cTextFillY );
        gfx::render_context_t trc( cTextBox, pColor );
        for( uint32_t yIdx = 0; yIdx < hmp::gfx::DIGIT_HEIGHT; yIdx++ ) {
            for( uint32_t xIdx = 0; xIdx < hmp::gfx::DIGIT_WIDTH; xIdx++ ) {
                const float64_t cDigitOffsetX = csDigitPaddingX + csDigitSpaceX * xIdx;
                const float64_t cDigitOffsetY = csDigitPaddingY + csDigitSpaceY * yIdx;

                if( cTextDigitMap[yIdx][xIdx] ) {
                    const box_t cDigitBox(
                        cDigitOffsetX, cDigitOffsetY,
                        csDigitFillX, csDigitFillY );
                    gfx::render_context_t drc( cDigitBox, pColor );
                    drc.render();
                }
            }
        }
    }
}

void vector::render( const vec2f32_t& pOrigin, const vec2f32_t& pDir, const float32_t pLength, const color4u8_t* pColor ) {
    const static float64_t csHeadRatio = 2.5 / 10.0;
    const static float64_t csTailRatio = 1.0 - csHeadRatio;
    const static float64_t csWidthRatio = 1.0 / 10.0;
    const static vec2f32_t csAxisI( 1.0f, 0.0f ), csAxisJ( 0.0f, 1.0f );

    const vec2f32_t cDirNorm = glm::normalize( pDir );
    const float32_t cDirAngle = glm::orientedAngle( csAxisI, cDirNorm );

    // GFX Environment Setup //

    glPushMatrix();
    glm::mat4 matVecSpace( 1.0f );
    matVecSpace *= glm::translate( glm::mat4(1.0f), glm::vec3(pOrigin.x, pOrigin.y, 0.0f) );
    matVecSpace *= glm::rotate( glm::mat4(1.0f), cDirAngle, glm::vec3(0.0f, 0.0f, 1.0f) );
    glMultMatrixf( &matVecSpace[0][0] );

    glPushAttrib( GL_CURRENT_BIT );
    glColor4ubv( (uint8_t*)pColor );

    { // Rendering //
        glBegin( GL_QUADS ); {
            glVertex2f( 0.0f, (-csWidthRatio / 2.0f) * pLength );
            glVertex2f( csTailRatio * pLength, (-csWidthRatio / 2.0f) * pLength );
            glVertex2f( csTailRatio * pLength, (csWidthRatio / 2.0f) * pLength );
            glVertex2f( 0.0f, (csWidthRatio / 2.0f) * pLength );
        } glEnd();

        glBegin( GL_TRIANGLES ); {
            glVertex2f( csTailRatio * pLength, (-csHeadRatio / 2.0f) * pLength );
            glVertex2f( (csTailRatio + csHeadRatio) * pLength, 0.0f );
            glVertex2f( csTailRatio * pLength, (csHeadRatio / 2.0f) * pLength );
        } glEnd();
    }

    // GFX Environment Teardown //
    
    glPopAttrib();
    glPopMatrix();
}

};

};
