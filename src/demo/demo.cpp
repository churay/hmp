#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_opengl_glext.h>

#include <cmath>
#include <cstring>

#include "gfx.h"

#include "demo_data.h"
#include "demo.h"

namespace demo {

/// Interface Functions ///

extern "C" bool32_t boot( demo::output_t* pOutput ) {
    // Initialize Graphics //

    // NOTE(JRC): The following code ensures that buffers have consistent aspect
    // ratios relative to their output spaces in screen space. This fact is crucial
    // in making code work in 'demo::gfx' related to fixing aspect ratios.
    pOutput->gfxBufferRess[demo::GFX_BUFFER_MASTER] = { 512, 512 };

    for( uint32_t gfxBufferIdx = 0; gfxBufferIdx < demo::GFX_BUFFER_COUNT; gfxBufferIdx++ ) {
        llce::gfx::fbo_t gfxBufferFBO( pOutput->gfxBufferRess[gfxBufferIdx] );

        LLCE_ASSERT_ERROR( gfxBufferFBO.valid(),
            "Failed to initialize frame buffer " << gfxBufferIdx << "; " <<
            "failed with frame buffer error " << glCheckFramebufferStatus(GL_FRAMEBUFFER) << "." );

        pOutput->gfxBufferFBOs[gfxBufferIdx] = gfxBufferFBO.mFrameID;
        pOutput->gfxBufferCBOs[gfxBufferIdx] = gfxBufferFBO.mColorID;
        pOutput->gfxBufferDBOs[gfxBufferIdx] = gfxBufferFBO.mDepthID;
    }

    return true;
}


extern "C" bool32_t init( demo::state_t* pState, demo::input_t* pInput ) {
    // Initialize Global Variables //

    pState->hsvColor = { 0.0f, 0.0f, 0.0f, 1.0f };

    // Initialize Input //

    std::memset( pInput, 0, sizeof(demo::input_t) );

    // Initialize Per-Mode Variables //

    return true;
}


extern "C" bool32_t update( demo::state_t* pState, demo::input_t* pInput, const demo::output_t* pOutput, const float64_t pDT ) {
    pState->hsvColor.x = std::fmod( 
        pState->hsvColor.x + demo::COLOR_VELOCITY * pDT, 360.0f );
    pState->hsvColor.y = demo::COLOR_SATURATION;
    pState->hsvColor.z = demo::COLOR_VALUE;
    pState->hsvColor.w = 1.0f;

    return true;
}


extern "C" bool32_t render( const demo::state_t* pState, const demo::input_t* pInput, const demo::output_t* pOutput ) {
    // NOTE(JRC): This code was adapted taken from this GitHub snippet:
    // https://gist.github.com/fairlight1337/4935ae72bcbcc1ba5c72
    const static auto hsv2rgb = [] ( const color4f32_t& hsv ) {
        color4f32_t rgb = { 0.0f, 0.0f, 0.0f, 1.0f };

        float fC = hsv.z * hsv.y; // Chroma
        float fHPrime = std::fmod( hsv.x / 60.0, 6 );
        float fX = fC * (1 - fabs(fmod(fHPrime, 2) - 1));
        float fM = hsv.z - fC;

        if(0 <= fHPrime && fHPrime < 1) {
            rgb.x = fC;
            rgb.y = fX;
            rgb.z = 0;
        } else if(1 <= fHPrime && fHPrime < 2) {
            rgb.x = fX;
            rgb.y = fC;
            rgb.z = 0;
        } else if(2 <= fHPrime && fHPrime < 3) {
            rgb.x = 0;
            rgb.y = fC;
            rgb.z = fX;
        } else if(3 <= fHPrime && fHPrime < 4) {
            rgb.x = 0;
            rgb.y = fX;
            rgb.z = fC;
        } else if(4 <= fHPrime && fHPrime < 5) {
            rgb.x = fX;
            rgb.y = 0;
            rgb.z = fC;
        } else if(5 <= fHPrime && fHPrime < 6) {
            rgb.x = fC;
            rgb.y = 0;
            rgb.z = fX;
        } else {
            rgb.x = 0;
            rgb.y = 0;
            rgb.z = 0;
        }

        rgb += fM * color4f32_t{ 1.0f, 1.0f, 1.0f, 0.0f };

        return rgb;
    };

    const static auto colorf2coloru = [] ( const color4f32_t& cf ) {
        color4u8_t cu;
        cu.x = std::floor( cf.x >= 1.0f ? 255 : cf.x * 256.0f );
        cu.y = std::floor( cf.y >= 1.0f ? 255 : cf.y * 256.0f );
        cu.z = std::floor( cf.z >= 1.0f ? 255 : cf.z * 256.0f );
        cu.w = std::floor( cf.w >= 1.0f ? 255 : cf.w * 256.0f );
        return cu;
    };

    llce::gfx::fbo_context_t metaFBOC(
        pOutput->gfxBufferFBOs[demo::GFX_BUFFER_MASTER],
        pOutput->gfxBufferRess[demo::GFX_BUFFER_MASTER] );

    color4f32_t rgbColor = hsv2rgb( pState->hsvColor );
    color4u8_t rgbColorByte = colorf2coloru( rgbColor );
    llce::gfx::render_context_t metaRC(
        llce::box_t(-1.0f, -1.0f, 2.0f, 2.0f),
        &rgbColorByte );
    metaRC.render();

    return true;
}

}
