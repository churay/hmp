#ifndef HMP_GFX_H
#define HMP_GFX_H

#include "hmp_box_t.h"
#include "hmp_consts.h"

namespace hmp {

namespace gfx {

struct render_context_t {
    render_context_t( const hmp::box_t& pBox, const color_t* pColor );
    ~render_context_t();

    void render() const;
};


struct fbo_context_t {
    fbo_context_t( const uint32_t pFBID, const uicoord32_t pFBRes );
    ~fbo_context_t();

    uicoord32_t mViewCoords, mViewRes;
};

};

};

#endif
