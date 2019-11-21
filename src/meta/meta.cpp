#include "meta.h"

/// Interface Functions ///

extern "C" bool32_t boot( meta::output_t* pOutput ) {
    // Initialize Graphics //

    vec2u32_t* gfxBuffRess = &pOutput->gfxBufferRess[0];
    meta::box_t* gfxBuffBoxs = &pOutput->gfxBufferBoxs[0];

    gfxBuffBoxs[meta::GFX_BUFFER_MASTER] = meta::box_t( 0.0f, 0.0f, 1.0f, 1.0f );
    gfxBuffBoxs[meta::GFX_BUFFER_SIM] = meta::box_t( 0.0f, 0.0f, 1.0f, 0.85f );
    gfxBuffBoxs[meta::GFX_BUFFER_UI] = meta::box_t( 0.0f, 0.85f, 1.0f, 0.15f );

    // NOTE(JRC): The following code ensures that buffers have consistent aspect
    // ratios relative to their output spaces in screen space. This fact is crucial
    // in making code work in 'meta::gfx' related to fixing aspect ratios.
    gfxBuffRess[meta::GFX_BUFFER_MASTER] = { 512, 512 };
    for( uint32_t gfxBufferIdx = meta::GFX_BUFFER_MASTER + 1; gfxBufferIdx < meta::GFX_BUFFER_COUNT; gfxBufferIdx++ ) {
        gfxBuffRess[gfxBufferIdx] = {
            (gfxBuffBoxs[gfxBufferIdx].mDims.x / gfxBuffBoxs[meta::GFX_BUFFER_MASTER].mDims.x) * gfxBuffRess[meta::GFX_BUFFER_MASTER].x,
            (gfxBuffBoxs[gfxBufferIdx].mDims.y / gfxBuffBoxs[meta::GFX_BUFFER_MASTER].mDims.y) * gfxBuffRess[meta::GFX_BUFFER_MASTER].y
        };
    }

    for( uint32_t gfxBufferIdx = 0; gfxBufferIdx < meta::GFX_BUFFER_COUNT; gfxBufferIdx++ ) {
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


extern "C" bool32_t init( meta::state_t* pState, meta::input_t* pInput ) {
    // Initialize Global Variables //

    pState->dts = llce::deque<float64_t, UI_FPS_FRAME_COUNT>( 0.0 );

    // Initialize Input //

    std::memset( pInput, 0, sizeof(meta::input_t) );

    // Initialize Per-Mode Variables //

    bool32_t initStatus = true;

    for( uint32_t modeIdx = 0; modeIdx < MODE_COUNT; modeIdx++ ) {
        initStatus &= MODE_INIT_FUNS[modeIdx]( pState );
    }

    return initStatus;
}


extern "C" bool32_t update( meta::state_t* pState, meta::input_t* pInput, const meta::output_t* pOutput, const float64_t pDT ) {
    dts.push_back( pDT );
    return true;
}


extern "C" bool32_t render( const meta::state_t* pState, const meta::input_t* pInput, const meta::output_t* pOutput ) {
    meta::gfx::render_context_t hmpRC( meta::box_t(-1.0f, -1.0f, 2.0f, 2.0f), &meta::color::BACKGROUND );
    hmpRC.render();

    bool32_t renderStatus = MODE_RENDER_FUNS[pState->mid]( pState, pInput, pOutput );
    pState->synth.render( pOutput->sfxConfig, pOutput->sfxBuffers[meta::SFX_BUFFER_MASTER] );
    return renderStatus;
}
