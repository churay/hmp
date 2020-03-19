#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_opengl_glext.h>

#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <cstring>

#include "gfx.h"

namespace llce {

namespace gfx {

/// 'llce::gfx::render_context_t' Functions ///

render_context_t::render_context_t( const box_t& pBox, const color4u8_t* pColor ) {
    glPushMatrix();
    glm::mat4 matModelWorld( 1.0f );
    matModelWorld *= glm::translate( glm::mat4(1.0f), vec3f32_t(pBox.mPos.x, pBox.mPos.y, 0.0f) );
    matModelWorld *= glm::scale( glm::mat4(1.0f), vec3f32_t(pBox.mDims.x, pBox.mDims.y, 1.0f) );
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
        vpMatrix = glm::translate( vpMatrix, vec3f32_t((vpCoords.x + vpRes.x) / 2.0f, (vpCoords.y + vpRes.y) / 2.0f, 0.5f) );
        vpMatrix = glm::scale( vpMatrix, vec3f32_t(vpRes.x / 2.0f, vpRes.y / 2.0f, 0.5f) );

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
    matRatio *= glm::translate( glm::mat4(1.0f), vec3f32_t(ratioBox.mPos.x, ratioBox.mPos.y, 0.0f) );
    matRatio *= glm::scale( glm::mat4(1.0f), vec3f32_t(ratioBox.mDims.x, ratioBox.mDims.y, 1.0f) );
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

/// 'llce::gfx::fbo_t' Functions ///

fbo_t::fbo_t( const vec2u32_t pFBRes ) {
    glGenFramebuffers( 1, &mFrameID );
    glBindFramebuffer( GL_FRAMEBUFFER, mFrameID );

    glGenTextures( 1, &mColorID );
    glBindTexture( GL_TEXTURE_2D, mColorID );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, pFBRes.x, pFBRes.y,
        0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, nullptr );
    glFramebufferTexture2D( GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mColorID, 0 );

    glGenTextures( 1, &mDepthID );
    glBindTexture( GL_TEXTURE_2D, mDepthID );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, pFBRes.x, pFBRes.y,
        0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr );
    glFramebufferTexture2D( GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mDepthID, 0 );
}


fbo_t::~fbo_t() {
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    glBindTexture( GL_TEXTURE_2D, 0 );
}


bool32_t fbo_t::valid() const {
    return glCheckFramebufferStatus( GL_FRAMEBUFFER ) == GL_FRAMEBUFFER_COMPLETE;
}

/// 'llce::gfx::fbo_context_t' Functions ///

fbo_context_t::fbo_context_t( const uint32_t pFBID, const vec2u32_t pFBRes ) {
    const static int32_t csContextFlags[] = { GL_VIEWPORT, GL_SCISSOR_BOX };
    const uint32_t cContextCount = ARRAY_LEN( csContextFlags );

    vec2i32_t* cContextParams[] = { &mViewport[0], &mScissor[0] };
    for( uint32_t contextIdx = 0; contextIdx < cContextCount; contextIdx++ ) {
        glGetIntegerv( csContextFlags[contextIdx],
            static_cast<int32_t*>(&(cContextParams[contextIdx]->x)) );
    }

    glBindFramebuffer( GL_FRAMEBUFFER, pFBID );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glViewport( 0, 0, pFBRes.x, pFBRes.y );
    glScissor( 0, 0, pFBRes.x, pFBRes.y );
}


fbo_context_t::~fbo_context_t() {
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    glViewport( mViewport[0].x, mViewport[0].y, mViewport[1].x, mViewport[1].y );
    glScissor( mScissor[0].x, mScissor[0].y, mScissor[1].x, mScissor[1].y );
}

/// 'llce::gfx::text' Functions ///

void text::render( const char8_t* pText, const color4u8_t* pColor ) {
    const static float64_t csDigitSpaceX = 1.0 / llce::gfx::DIGIT_WIDTH;
    const static float64_t csDigitSpaceY = 1.0 / llce::gfx::DIGIT_HEIGHT;
    const static float64_t csDigitPaddingX = csDigitSpaceX / 10.0;
    const static float64_t csDigitPaddingY = csDigitSpaceY / 10.0;
    const static float64_t csDigitFillX = csDigitSpaceX - 2.0 * csDigitPaddingX;
    const static float64_t csDigitFillY = csDigitSpaceY - 2.0 * csDigitPaddingY;

    const uint32_t cTextLength = std::strlen( pText );
    const float64_t cTextSpacingX = 2.0 * llce::gfx::DIGIT_WIDTH;
    const float64_t cTextSpacingY = 0.0;
    const float64_t cTextFillX = cTextSpacingX / ( (cTextSpacingX + 1.0) * cTextLength - 1.0 );
    const float64_t cTextFillY = 1.0;

    const box_t cRenderBox( 0.0f, 0.0f, 1.0f, 1.0f );
    gfx::render_context_t rrc( cRenderBox, llce::gfx::DIGIT_ASPECT * cTextLength, pColor );
    for( const char8_t* pTextItr = pText; *pTextItr != '\0'; pTextItr++ ) {
        const auto cTextDigitMap = &llce::gfx::ASCII_DIGIT_MAP[static_cast<uint32_t>(*pTextItr)][0];
        const uint32_t cTextIdx = pTextItr - pText;

        const float64_t cTextOffsetX = ( cTextFillX + cTextFillX / cTextSpacingX ) * cTextIdx;
        const float64_t cTextOffsetY = 0.0;

        const box_t cTextBox( cTextOffsetX, cTextOffsetY, cTextFillX, cTextFillY );
        gfx::render_context_t trc( cTextBox, pColor );
        for( uint32_t yIdx = 0; yIdx < llce::gfx::DIGIT_HEIGHT; yIdx++ ) {
            for( uint32_t xIdx = 0; xIdx < llce::gfx::DIGIT_WIDTH; xIdx++ ) {
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

/// 'llce::gfx::vector' Functions ///

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
    matVecSpace *= glm::translate( glm::mat4(1.0f), vec3f32_t(pOrigin.x, pOrigin.y, 0.0f) );
    matVecSpace *= glm::rotate( glm::mat4(1.0f), cDirAngle, vec3f32_t(0.0f, 0.0f, 1.0f) );
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

/// 'llce::gfx::circle' Functions ///

void circle::render( const circle_t& pCircle, const color4u8_t* pColor ) {
    circle::render( pCircle, 0.0f, 2.0f * glm::pi<float32_t>(), pColor );
}


void circle::render( const circle_t& pCircle, const float32_t pStartRadians, const float32_t pEndRadians, const color4u8_t* pColor ) {
    const static uint32_t csSegmentsPer2PI = 20;

    // GFX Environment Setup //

    glPushMatrix();
    glm::mat4 matCircleSpace( 1.0f );
    matCircleSpace *= glm::translate( glm::mat4(1.0f), vec3f32_t(pCircle.mCenter.x, pCircle.mCenter.y, 0.0f) );
    matCircleSpace *= glm::scale( glm::mat4(1.0f), glm::vec3(pCircle.mRadius / 2.0f, pCircle.mRadius / 2.0f, 1.0f) );
    glMultMatrixf( &matCircleSpace[0][0] );

    glPushAttrib( GL_CURRENT_BIT );
    glColor4ubv( (uint8_t*)pColor );

    // FIXME(JRC): The following code crashes for very small intervals as they cause
    // the interpolation scheme to divide by zero.
    { // Rendering //
        const interval_t cRadianInterval( pStartRadians, pEndRadians );
        const uint32_t cSegmentCount = std::ceil( cRadianInterval.length() * csSegmentsPer2PI );
        glBegin( GL_TRIANGLE_FAN );
        glVertex2f( 0.0f, 0.0f );
        for( uint32_t segmentIdx = 0; segmentIdx < cSegmentCount; segmentIdx++ ) {
            float32_t segmentRadians = cRadianInterval.interp( segmentIdx / (cSegmentCount - 1.0f) );
            glVertex2f( std::cos(segmentRadians), std::sin(segmentRadians) );
        }
        glEnd();
    }

    // GFX Environment Teardown //

    glPopAttrib();
    glPopMatrix();
}

/// 'llce::gfx' Constants ///

const bool8_t ASCII_DIGIT_MAP[128][DIGIT_HEIGHT][DIGIT_WIDTH] = {
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'NUL'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'SOH'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'STX'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'ETX'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'EOT'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'ENQ'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'ACK'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'BEL'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'BS'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'TAB'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'LF'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'VT'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'FF'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'CR'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'SO'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'SI'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'DLE'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'DC1'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'DC2'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'DC3'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'DC4'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'NAK'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'SYN'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'ETB'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'CAN'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'EM'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'SUB'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'ESC'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'FS'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'GS'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'RS'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'US'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // ' '
    { {0, 1, 1, 0, 0}, {0, 1, 1, 0, 0}, {0, 0, 0, 0, 0}, {0, 1, 1, 0, 0}, {0, 1, 1, 0, 0}, {0, 1, 1, 0, 0}, {0, 1, 1, 0, 0} }, // '!'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // '"'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // '#'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // '$'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // '%'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // '&'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // '''
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // '('
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // ')'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // '*'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // '+'
    { {0, 1, 0, 0, 0}, {0, 0, 1, 0, 0}, {0, 1, 1, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // ','
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // '-'
    { {0, 0, 0, 0, 0}, {0, 1, 1, 0, 0}, {0, 1, 1, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // '.'
    { {1, 1, 0, 0, 0}, {1, 1, 0, 0, 0}, {0, 1, 1, 0, 0}, {0, 0, 1, 0, 0}, {0, 0, 1, 1, 0}, {0, 0, 0, 1, 1}, {0, 0, 0, 1, 1} }, // '/'
    { {0, 1, 1, 1, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 1, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {0, 1, 1, 1, 0} }, // '0'
    { {1, 1, 1, 1, 1}, {0, 0, 1, 0, 0}, {0, 0, 1, 0, 0}, {0, 0, 1, 0, 0}, {1, 0, 1, 0, 0}, {0, 1, 1, 0, 0}, {0, 0, 1, 0, 0} }, // '1'
    { {1, 1, 1, 1, 1}, {1, 0, 0, 0, 0}, {0, 1, 0, 0, 0}, {0, 0, 1, 1, 0}, {0, 0, 0, 0, 1}, {0, 0, 0, 0, 1}, {1, 1, 1, 1, 0} }, // '2'
    { {1, 1, 1, 1, 0}, {0, 0, 0, 0, 1}, {0, 0, 0, 0, 1}, {0, 1, 1, 1, 0}, {0, 0, 0, 0, 1}, {0, 0, 0, 0, 1}, {1, 1, 1, 1, 0} }, // '3'
    { {0, 0, 0, 0, 1}, {0, 0, 0, 0, 1}, {1, 1, 1, 1, 1}, {0, 1, 0, 0, 1}, {0, 0, 1, 0, 1}, {0, 0, 0, 1, 1}, {0, 0, 0, 0, 1} }, // '4'
    { {1, 1, 1, 1, 0}, {0, 0, 0, 0, 1}, {0, 0, 0, 0, 1}, {1, 1, 1, 1, 0}, {1, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {1, 1, 1, 1, 1} }, // '5'
    { {0, 1, 1, 1, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 1, 0, 0, 1}, {1, 0, 1, 1, 0}, {1, 0, 0, 0, 0}, {0, 1, 1, 1, 1} }, // '6'
    { {1, 0, 0, 0, 0}, {0, 1, 0, 0, 0}, {0, 0, 1, 0, 0}, {0, 0, 0, 1, 0}, {0, 0, 0, 0, 1}, {0, 0, 0, 0, 1}, {1, 1, 1, 1, 1} }, // '7'
    { {0, 1, 1, 1, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {0, 1, 1, 1, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {0, 1, 1, 1, 0} }, // '8'
    { {1, 1, 1, 1, 0}, {0, 0, 0, 0, 1}, {0, 1, 1, 0, 1}, {1, 0, 0, 1, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {0, 1, 1, 1, 0} }, // '9'
    { {0, 0, 0, 0, 0}, {0, 1, 1, 0, 0}, {0, 1, 1, 0, 0}, {0, 0, 0, 0, 0}, {0, 1, 1, 0, 0}, {0, 1, 1, 0, 0}, {0, 0, 0, 0, 0} }, // ':'
    { {0, 1, 0, 0, 0}, {0, 0, 1, 0, 0}, {0, 1, 1, 0, 0}, {0, 0, 0, 0, 0}, {0, 1, 1, 0, 0}, {0, 1, 1, 0, 0}, {0, 0, 0, 0, 0} }, // ';'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // '<'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // '='
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // '>'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // '?'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // '@'
    { {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 1, 1, 1, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {0, 1, 1, 1, 0} }, // 'A'
    { {1, 1, 1, 1, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 1, 1, 1, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 1, 1, 1, 0} }, // 'B'
    { {0, 1, 1, 1, 1}, {1, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {0, 1, 1, 1, 1} }, // 'C'
    { {1, 1, 1, 1, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 1, 1, 1, 0} }, // 'D'
    { {1, 1, 1, 1, 1}, {1, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {1, 1, 1, 1, 0}, {1, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {1, 1, 1, 1, 1} }, // 'E'
    { {1, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {1, 1, 1, 1, 0}, {1, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {1, 1, 1, 1, 1} }, // 'F'
    { {0, 1, 1, 1, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 1, 1, 1}, {1, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {0, 1, 1, 1, 1} }, // 'G'
    { {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 1, 1, 1, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1} }, // 'H'
    { {1, 1, 1, 1, 1}, {0, 0, 1, 0, 0}, {0, 0, 1, 0, 0}, {0, 0, 1, 0, 0}, {0, 0, 1, 0, 0}, {0, 0, 1, 0, 0}, {1, 1, 1, 1, 1} }, // 'I'
    { {0, 1, 1, 0, 0}, {1, 0, 0, 1, 0}, {1, 0, 0, 1, 0}, {0, 0, 0, 1, 0}, {0, 0, 0, 1, 0}, {0, 0, 0, 1, 0}, {0, 0, 0, 1, 0} }, // 'J'
    { {1, 0, 0, 0, 1}, {1, 0, 0, 1, 0}, {1, 0, 1, 0, 0}, {1, 1, 0, 0, 0}, {1, 0, 1, 0, 0}, {1, 0, 0, 1, 0}, {1, 0, 0, 0, 1} }, // 'K'
    { {1, 1, 1, 1, 1}, {1, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {1, 0, 0, 0, 0} }, // 'L'
    { {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 1, 0, 1}, {1, 0, 1, 0, 1}, {1, 1, 0, 1, 1}, {1, 0, 0, 0, 1} }, // 'M'
    { {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 1, 1}, {1, 0, 1, 0, 1}, {1, 1, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1} }, // 'N'
    { {0, 1, 1, 1, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {0, 1, 1, 1, 0} }, // 'O'
    { {1, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {1, 1, 1, 1, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 1, 1, 1, 0} }, // 'P'
    { {0, 1, 1, 1, 1}, {1, 0, 0, 1, 1}, {1, 0, 1, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {0, 1, 1, 1, 0} }, // 'Q'
    { {1, 0, 0, 0, 1}, {1, 0, 0, 1, 0}, {1, 0, 1, 0, 0}, {1, 1, 1, 1, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 1, 1, 1, 0} }, // 'R'
    { {1, 1, 1, 1, 0}, {0, 0, 0, 0, 1}, {0, 0, 0, 0, 1}, {0, 1, 1, 1, 0}, {1, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {0, 1, 1, 1, 1} }, // 'S'
    { {0, 0, 1, 0, 0}, {0, 0, 1, 0, 0}, {0, 0, 1, 0, 0}, {0, 0, 1, 0, 0}, {0, 0, 1, 0, 0}, {0, 0, 1, 0, 0}, {1, 1, 1, 1, 1} }, // 'T'
    { {0, 1, 1, 1, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1} }, // 'U'
    { {0, 0, 1, 0, 0}, {0, 1, 0, 1, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1} }, // 'V'
    { {1, 0, 0, 0, 1}, {1, 1, 0, 1, 1}, {1, 0, 1, 0, 1}, {1, 0, 1, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1} }, // 'W'
    { {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {0, 1, 0, 1, 0}, {0, 0, 1, 0, 0}, {0, 1, 0, 1, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1} }, // 'X'
    { {0, 0, 1, 0, 0}, {0, 0, 1, 0, 0}, {0, 0, 1, 0, 0}, {0, 0, 1, 0, 0}, {0, 1, 0, 1, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1} }, // 'Y'
    { {1, 1, 1, 1, 1}, {1, 0, 0, 0, 0}, {0, 1, 0, 0, 0}, {0, 0, 1, 0, 0}, {0, 0, 0, 1, 0}, {0, 0, 0, 0, 1}, {1, 1, 1, 1, 1} }, // 'Z'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // '['
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // '\'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // ']'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // '^'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // '_'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // '`'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'a'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'b'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'c'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'd'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'e'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'f'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'g'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'h'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'i'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'j'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'k'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'l'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'm'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'n'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'o'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'p'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'q'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'r'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 's'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 't'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'u'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'v'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'w'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'x'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'y'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // 'z'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // '{'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // '|'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // '}'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // '~'
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} }, // ''
};

};

};
