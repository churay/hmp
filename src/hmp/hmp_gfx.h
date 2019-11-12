#ifndef HMP_GFX_H
#define HMP_GFX_H

#include "hmp_box_t.h"
#include "hmp_consts.h"

namespace hmp {

namespace gfx {

struct render_context_t {
    render_context_t( const hmp::box_t& pBox, const color4u8_t* pColor );
    render_context_t( const hmp::box_t& pBox, const float32_t pScreenRatio, const color4u8_t* pColor );
    ~render_context_t();

    void render() const;
};


struct fbo_context_t {
    fbo_context_t( const uint32_t pFBID, const vec2u32_t pFBRes );
    ~fbo_context_t();

    vec2i32_t mViewport[2], mScissor[2];
};

namespace text {
    void render( const char8_t* pText, const color4u8_t* pColor );
};

namespace vector {
    void render( const vec2f32_t& pOrigin, const vec2f32_t& pDir, const float32_t pLength, const color4u8_t* pColor );
};

};

};

#endif
