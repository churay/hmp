#ifndef LLCE_OUTPUT_H
#define LLCE_OUTPUT_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_opengl_glext.h>

#include <array>

#include "gfx.h"
#include "sfx.h"

#include "consts.h"

namespace llce {

namespace output {

/// Namespace Attributes ///

// NOTE(JRC): This is the ID used for the various shared asset buffers that
// are passed between the 'llce' harness and the current simulation.
constexpr static uint32_t BUFFER_SHARED_ID = 0;

// TODO(JRC): This should be cleaned up if at all possible to reduce the amount
// of code duplication between templates.

template <uint32_t GFXBuffers, uint32_t SFXBuffers> struct output_t {
    // Graphics Output //
    uint32_t gfxBufferFBOs[GFXBuffers];   // frame buffers
    uint32_t gfxBufferCBOs[GFXBuffers];   // color buffers
    uint32_t gfxBufferDBOs[GFXBuffers];   // depth buffers
    vec2u32_t gfxBufferRess[GFXBuffers];  // buffer resolutions
    box_t gfxBufferBoxs[GFXBuffers];      // buffer locations

    // Audio Output //
    SDL_AudioSpec sfxConfig;
    bit8_t* sfxBuffers[SFXBuffers];
    uint32_t sfxBufferFrames[SFXBuffers];
};
template <> struct output_t<0, 0> {};
template <uint32_t GFXBuffers> struct output_t<GFXBuffers, 0> {
    uint32_t gfxBufferFBOs[GFXBuffers];
    uint32_t gfxBufferCBOs[GFXBuffers];
    uint32_t gfxBufferDBOs[GFXBuffers];
    vec2u32_t gfxBufferRess[GFXBuffers];
    box_t gfxBufferBoxs[GFXBuffers];
};
template <uint32_t SFXBuffers> struct output_t<0, SFXBuffers> { 
    SDL_AudioSpec sfxConfig;
    bit8_t* sfxBuffers[SFXBuffers];
    uint32_t sfxBufferFrames[SFXBuffers];
};

/// Namespace Functions ///

template <uint32_t GFXBuffers, uint32_t SFXBuffers>
void gfxboot( output_t<GFXBuffers, SFXBuffers>& pOutput, const vec2u32_t& pGFXBufferRes,
        const std::array<box_t, GFXBuffers>& pGFXBufferBoxs = std::array<box_t, GFXBuffers>() ) {
    const static llce::box_t csFullBox( 0.0f, 0.0f, 1.0f, 1.0f );

    std::array<box_t, GFXBuffers> gfxBufferBoxs;
    for( uint32_t bufferIdx = 0; bufferIdx < GFXBuffers; bufferIdx++ ) {
        gfxBufferBoxs[bufferIdx] =
            ( pGFXBufferBoxs[bufferIdx].valid() && bufferIdx != BUFFER_SHARED_ID ) ?
            pGFXBufferBoxs[bufferIdx] : csFullBox;
        LLCE_CHECK_ERROR( csFullBox.contains(gfxBufferBoxs[bufferIdx]),
            "Invalid buffer boundaries specified for frame buffer " << bufferIdx << "; " <<
            "boundaries must be expressed in terms of percentage of the unit cube." );
    }

    // NOTE(JRC): The following code ensures that buffers have consistent aspect
    // ratios relative to their output spaces in screen space. This fact is crucial
    // in making code work in 'ssn::gfx' related to fixing aspect ratios.
    for( uint32_t bufferIdx = 0; bufferIdx < GFXBuffers; bufferIdx++ ) {
        pOutput.gfxBufferBoxs[bufferIdx] = gfxBufferBoxs[bufferIdx];
        pOutput.gfxBufferRess[bufferIdx] = {
            (gfxBufferBoxs[bufferIdx].mDims.x / csFullBox.mDims.x) * pGFXBufferRes.x,
            (gfxBufferBoxs[bufferIdx].mDims.y / csFullBox.mDims.y) * pGFXBufferRes.y
        };
    }

    for( uint32_t bufferIdx = 0; bufferIdx < GFXBuffers; bufferIdx++ ) {
        llce::gfx::fbo_t gfxBufferFBO( pOutput.gfxBufferRess[bufferIdx] );
        LLCE_ASSERT_ERROR( gfxBufferFBO.valid(),
            "Failed to initialize frame buffer " << bufferIdx << "; " <<
            "failed with frame buffer error '" << glCheckFramebufferStatus(GL_FRAMEBUFFER) << "'." );

        pOutput.gfxBufferFBOs[bufferIdx] = gfxBufferFBO.mFrameID;
        pOutput.gfxBufferCBOs[bufferIdx] = gfxBufferFBO.mColorID;
        pOutput.gfxBufferDBOs[bufferIdx] = gfxBufferFBO.mDepthID;
    }
}

}

}

#endif

