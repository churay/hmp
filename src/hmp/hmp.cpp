#include <cstring>
#include <cstdlib>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_opengl_glext.h>

#include <glm/geometric.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "hmp_modes.h"
#include "hmp_gfx.h"
#include "hmp_sfx.h"
#include "hmp_data.h"
#include "hmp.h"

/// Global Declarations ///

typedef bool32_t (*init_f)( hmp::state_t* );
typedef bool32_t (*update_f)( hmp::state_t*, hmp::input_t*, const float64_t );
typedef bool32_t (*render_f)( const hmp::state_t*, const hmp::input_t*, const hmp::output_t* );

/// Per-Mode Tables ///

constexpr static init_f MODE_INIT_FUNS[] = {
    hmp::mode::game::init, hmp::mode::menu::init, hmp::mode::reset::init };
constexpr static update_f MODE_UPDATE_FUNS[] = {
    hmp::mode::game::update, hmp::mode::menu::update, hmp::mode::reset::update };
constexpr static render_f MODE_RENDER_FUNS[] = {
    hmp::mode::game::render, hmp::mode::menu::render, hmp::mode::reset::render };
constexpr static uint32_t MODE_COUNT = ARRAY_LEN( MODE_INIT_FUNS );

/// Interface Functions ///

extern "C" bool32_t boot( hmp::output_t* pOutput ) {
    // Initialize Graphics //

    vec2u32_t* gfxBuffRess = &pOutput->gfxBufferRess[0];
    hmp::box_t* gfxBuffBoxs = &pOutput->gfxBufferBoxs[0];

    gfxBuffBoxs[hmp::GFX_BUFFER_MASTER] = hmp::box_t( 0.0f, 0.0f, 1.0f, 1.0f );
    gfxBuffBoxs[hmp::GFX_BUFFER_SIM] = hmp::box_t( 0.0f, 0.0f, 1.0f, 0.85f );
    gfxBuffBoxs[hmp::GFX_BUFFER_UI] = hmp::box_t( 0.0f, 0.85f, 1.0f, 0.15f );

    // NOTE(JRC): The following code ensures that buffers have consistent aspect
    // ratios relative to their output spaces in screen space. This fact is crucial
    // in making code work in 'hmp::gfx' related to fixing aspect ratios.
    gfxBuffRess[hmp::GFX_BUFFER_MASTER] = { 512, 512 };
    for( uint32_t gfxBufferIdx = hmp::GFX_BUFFER_MASTER + 1; gfxBufferIdx < hmp::GFX_BUFFER_COUNT; gfxBufferIdx++ ) {
        gfxBuffRess[gfxBufferIdx] = {
            (gfxBuffBoxs[gfxBufferIdx].mDims.x / gfxBuffBoxs[hmp::GFX_BUFFER_MASTER].mDims.x) * gfxBuffRess[hmp::GFX_BUFFER_MASTER].x,
            (gfxBuffBoxs[gfxBufferIdx].mDims.y / gfxBuffBoxs[hmp::GFX_BUFFER_MASTER].mDims.y) * gfxBuffRess[hmp::GFX_BUFFER_MASTER].y
        };
    }

    for( uint32_t gfxBufferIdx = 0; gfxBufferIdx < hmp::GFX_BUFFER_COUNT; gfxBufferIdx++ ) {
        uint32_t& gfxBufferFBO = pOutput->gfxBufferFBOs[gfxBufferIdx];
        uint32_t& gfxBufferCBO = pOutput->gfxBufferCBOs[gfxBufferIdx];
        uint32_t& gfxBufferDBO = pOutput->gfxBufferDBOs[gfxBufferIdx];
        const vec2u32_t& gfxBufferRes = pOutput->gfxBufferRess[gfxBufferIdx];

        glGenFramebuffers( 1, &gfxBufferFBO );
        glBindFramebuffer( GL_FRAMEBUFFER, gfxBufferFBO );

        glGenTextures( 1, &gfxBufferCBO );
        glBindTexture( GL_TEXTURE_2D, gfxBufferCBO );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, gfxBufferRes.x, gfxBufferRes.y,
            0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, nullptr );
        glFramebufferTexture2D( GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gfxBufferCBO, 0 );

        glGenTextures( 1, &gfxBufferDBO );
        glBindTexture( GL_TEXTURE_2D, gfxBufferDBO );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, gfxBufferRes.x, gfxBufferRes.y,
            0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr );
        glFramebufferTexture2D( GL_FRAMEBUFFER,
            GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gfxBufferDBO, 0 );

        LLCE_ASSERT_ERROR(
            glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
            "Failed to initialize HMP frame buffer " << gfxBufferIdx << "; " <<
            "failed with frame buffer error " << glCheckFramebufferStatus(GL_FRAMEBUFFER) << "." );
    }

    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    glBindTexture( GL_TEXTURE_2D, 0 );

    return true;
}


extern "C" bool32_t init( hmp::state_t* pState, hmp::input_t* pInput ) {
    // Initialize Global Variables //

    pState->dt = 0.0;
    pState->tt = 0.0;

    pState->mid = hmp::mode::boot_id;
    pState->pmid = hmp::mode::menu_id;

    pState->rng = hmp::rng_t( hmp::RNG_SEED );
    pState->synth = hmp::sfx::synth_t();

    // Initialize Input //

    std::memset( pInput, 0, sizeof(hmp::input_t) );

    // Initialize Per-Mode Variables //

    bool32_t initStatus = true;

    for( uint32_t modeIdx = 0; modeIdx < MODE_COUNT; modeIdx++ ) {
        initStatus &= MODE_INIT_FUNS[modeIdx]( pState );
    }

    return initStatus;
}


extern "C" bool32_t update( hmp::state_t* pState, hmp::input_t* pInput, const hmp::output_t* pOutput, const float64_t pDT ) {
    if( pState->mid != pState->pmid ) {
        if( pState->pmid < 0 ) { return false; }
        MODE_INIT_FUNS[pState->pmid]( pState );
        pState->mid = pState->pmid;
    }

    pState->dt = pDT;
    pState->tt += pDT;

    bool32_t updateStatus = MODE_UPDATE_FUNS[pState->mid]( pState, pInput, pDT );
    pState->synth.update( pDT, pOutput->sfxBufferFrames[hmp::SFX_BUFFER_MASTER] );
    return updateStatus;
}


extern "C" bool32_t render( const hmp::state_t* pState, const hmp::input_t* pInput, const hmp::output_t* pOutput ) {
    hmp::gfx::render_context_t hmpRC( hmp::box_t(-1.0f, -1.0f, 2.0f, 2.0f), &hmp::color::BACKGROUND );
    hmpRC.render();

    bool32_t renderStatus = MODE_RENDER_FUNS[pState->mid]( pState, pInput, pOutput );
    pState->synth.render( pOutput->sfxConfig, pOutput->sfxBuffers[hmp::SFX_BUFFER_MASTER] );
    return renderStatus;
}
