#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_opengl_glext.h>

#include <cstring>

#include "input.h"
#include "output.h"
#include "gfx.h"

#include "meta.h"

namespace meta {

/// Interface Functions ///

bool32_t boot( meta::output_t* pOutput ) {
    // Initialize Graphics //

    const vec2u32_t cGFXBuffRes( 512, 128 );

    llce::output::boot<1, 0>( *pOutput, cGFXBuffRes );

    // Initialize Sound //

    // ... //

    return true;
}


bool32_t init( meta::state_t* pState, meta::input_t* pInput ) {
    // Initialize Global Variables //

    pState->dts = llce::deque<float64_t, UI_FPS_FRAME_COUNT>();
    pState->dts.push_back( 0.0 );

    // Initialize Input //

    std::memset( pInput, 0, sizeof(meta::input_t) );

    // Initialize Per-Mode Variables //

    return true;
}


bool32_t update( meta::state_t* pState, meta::input_t* pInput, const meta::output_t* pOutput, const float64_t pDT ) {
    pState->dts.push_back( pDT );
    return true;
}


bool32_t render( const meta::state_t* pState, const meta::input_t* pInput, const meta::output_t* pOutput ) {
    llce::gfx::fbo_context_t metaFBOC(
        pOutput->gfxBufferFBOs[llce::output::BUFFER_SHARED_ID],
        pOutput->gfxBufferRess[llce::output::BUFFER_SHARED_ID] );

    llce::gfx::render_context_t metaRC( llce::box_t(-1.0f, -1.0f, 2.0f, 2.0f) );
    llce::gfx::color_context_t metaCC( &meta::UI_FPS_BACKGROUND_COLOR );
    llce::gfx::render::box();

    const static float32_t csMetaUILineWidth = 5.0f;
    const static vec2f32_t csMetaUITargetPadding = { 0.0f, 0.25f };

    { // Render Trend Line //
        const static color4u8_t csMetaUITrendColor = { 0xFF, 0x00, 0x00, 0xFF };
        const static uint16_t csMetaUITrendPattern = 0xFFFF;

        llce::gfx::render_context_t trendLineRC(
            llce::box_t(0.0f, 0.0f, 1.0f, 1.0f - csMetaUITargetPadding.y) );
        llce::gfx::color_context_t trendLineCC( &csMetaUITrendColor );

        glLineWidth( csMetaUILineWidth );
        glLineStipple( 1, csMetaUITrendPattern );
        glBegin( GL_LINE_STRIP ); {
            for( uint32_t frameIdx = 0; frameIdx < pState->dts.size(); frameIdx++ ) {
                float64_t frameU = 1.0f - ( (frameIdx + 0.0f) / pState->dts.capacity() );
                float64_t frameV = ( 1.0 / pState->dts.back(frameIdx) ) / LLCE_FPS;
                glVertex2f( frameU, frameV );
            }
        } glEnd();
    }

    { // Render UI Elements //
        const static color4u8_t csMetaUITargetColor = { 0x00, 0x00, 0xFF, 0xFF };
        const static uint16_t csMetaUITargetPattern = 0x00FF;

        llce::gfx::color_context_t targetLineCC( &csMetaUITargetColor );

        glLineWidth( csMetaUILineWidth );
        glLineStipple( 1, csMetaUITargetPattern );
        glBegin( GL_LINES ); {
            glVertex2f( 0.0f - csMetaUITargetPadding.x, 1.0f - csMetaUITargetPadding.y );
            glVertex2f( 1.0f + csMetaUITargetPadding.x, 1.0f - csMetaUITargetPadding.y );
        } glEnd();
    }

    return true;
}

}
