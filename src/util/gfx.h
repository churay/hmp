#ifndef LLCE_GFX_H
#define LLCE_GFX_H

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "box_t.h"
#include "circle_t.h"
#include "consts.h"

namespace llce {

namespace gfx {

/// Namespace Attributes ///

constexpr static uint32_t DIGIT_WIDTH = 5, DIGIT_HEIGHT = 7;
constexpr static float64_t DIGIT_ASPECT = ( DIGIT_WIDTH + 0.0 ) / ( DIGIT_HEIGHT + 0.0 );
const extern bool8_t ASCII_DIGIT_MAP[128][DIGIT_HEIGHT][DIGIT_WIDTH];

/// Namespace Types ///

struct color_context_t {
    color_context_t( const color4u8_t* pColor );
    color_context_t( const color4f32_t* pColor );
    ~color_context_t();

    void update( const color4u8_t* pColor );
    void update( const color4f32_t* pColor );
};


struct render_context_t {
    render_context_t( const box_t& pBox );
    render_context_t( const box_t& pBox, const float32_t pScreenRatio );
    render_context_t( const vec2f32_t& pPos, const vec2f32_t& pBasisX, const vec2f32_t& pBasisY );
    ~render_context_t();
};


struct fbo_t {
    fbo_t( const vec2u32_t pFBRes );
    ~fbo_t();

    bool32_t valid() const;

    uint32_t mFrameID, mColorID, mDepthID;
};


struct fbo_context_t {
    fbo_context_t( const uint32_t pFBID, const vec2u32_t pFBRes );
    ~fbo_context_t();

    vec2i32_t mViewport[2], mScissor[2];
};

/// Namespace Functions ///

float32_t aspect( const vec2i32_t& pDims );
float32_t aspect( const vec2u32_t& pDims );
float32_t aspect( const vec2f32_t& pDims );

mat4f32_t glMatrix();

namespace color {
    color4f32_t u82f32( const color4u8_t& pColor );
    color4u8_t f322u8( const color4f32_t& pColor );

    color4f32_t rgb2hsv( const color4f32_t& pColorRGB );
    color4f32_t hsv2rgb( const color4f32_t& pColorHSV );

    color4u8_t transparentize( const color4u8_t& pColor, const float32_t pPercent );
    color4f32_t transparentize( const color4f32_t& pColor, const float32_t pPercent );

    color4f32_t saturateRGB( const color4f32_t& pColorRGB, const float32_t pPercent );
    color4f32_t saturateHSV( const color4f32_t& pColorHSV, const float32_t pPercent );
};

namespace render {
    void vector( const vec2f32_t& pOrigin, const vec2f32_t& pDir, const float32_t pLength );

    void box( const box_t& pBox = box_t(0.0f, 0.0f, 1.0f, 1.0f) );
    void box( const vec2f32_t& pSize, const vec2f32_t& pPos,
        const llce::geom::anchor2D_e pAnchor = llce::geom::anchor2D::ll );

    void circle( const circle_t& pCircle,
        const float32_t pStartRadians = 0.0f,
        const float32_t pEndRadians = glm::two_pi<float32_t>() );

    void border( const float32_t (&pSizes)[4] );

    void text( const char8_t* pText,
        const box_t& pRenderBox = box_t(0.0f, 0.0f, 1.0f, 1.0f) );
    void text( const char8_t* pText, const float32_t pSize, const vec2f32_t& pPos,
        const llce::geom::anchor2D_e pAnchor = llce::geom::anchor2D::ll );
};

};

};

#endif
