#ifndef LLCE_GFX_H
#define LLCE_GFX_H

#include "box_t.h"
#include "circle_t.h"
#include "consts.h"

namespace llce {

namespace gfx {

constexpr static uint32_t DIGIT_WIDTH = 5, DIGIT_HEIGHT = 7;
constexpr static float64_t DIGIT_ASPECT = ( DIGIT_WIDTH + 0.0 ) / ( DIGIT_HEIGHT + 0.0 );
const extern bool8_t ASCII_DIGIT_MAP[128][DIGIT_HEIGHT][DIGIT_WIDTH];

struct render_context_t {
    render_context_t( const box_t& pBox, const color4u8_t* pColor );
    render_context_t( const box_t& pBox, const float32_t pScreenRatio, const color4u8_t* pColor );
    render_context_t( const vec2f32_t& pPos, const vec2f32_t& pBasisX, const vec2f32_t& pBasisY, const color4u8_t* pColor );
    ~render_context_t();

    void render() const;
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

namespace color {
    color4f32_t u82f32( const color4u8_t& pColor );
    color4u8_t f322u8( const color4f32_t& pColor );

    color4f32_t rgb2hsv( const color4f32_t& pColorRGB );
    color4f32_t hsv2rgb( const color4f32_t& pColorHSV );

    color4f32_t saturateRGB( const color4f32_t& pColorRGB, const float32_t pPercent );
    color4f32_t saturateHSV( const color4f32_t& pColorHSV, const float32_t pPercent );
};

namespace text {
    void render( const char8_t* pText, const color4u8_t* pColor );
};

namespace vector {
    void render( const vec2f32_t& pOrigin, const vec2f32_t& pDir, const float32_t pLength, const color4u8_t* pColor );
};

namespace circle {
    void render( const circle_t& pCircle, const color4u8_t* pColor );
    void render( const circle_t& pCircle, const float32_t pStartRadians, const float32_t pEndRadians, const color4u8_t* pColor );
};

};

};

#endif
