#ifndef HMP_GFX_H
#define HMP_GFX_H

#include "hmp_box_t.h"
#include "hmp_consts.h"

namespace hmp {

namespace gfx {

struct render_context_t {
    render_context_t( const hmp::box_t& pBox, const color32_t* pColor );
    ~render_context_t();

    void render() const;
};


struct fbo_context_t {
    fbo_context_t( const uint32_t pFBID, const vec2u32_t pFBRes );
    ~fbo_context_t();

    vec2u32_t mViewCoords, mViewRes;
};

namespace text {

constexpr static uint32_t DIGIT_WIDTH = 5, DIGIT_HEIGHT = 7;
constexpr static float64_t DIGIT_ASPECT = ( DIGIT_WIDTH + 0.0 ) / ( DIGIT_HEIGHT + 0.0 );
const extern bool8_t ASCII_DIGIT_MAP[128][DIGIT_HEIGHT][DIGIT_WIDTH];

void render( const char8_t* pText, const color32_t* pColor );

};

};

};

#endif
