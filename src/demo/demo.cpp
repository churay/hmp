#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_opengl_glext.h>

#include <glm/common.hpp>

#include <cmath>
#include <cstring>
#include <limits>

#include "sfx.h"
#include "input.h"
#include "output.h"

#include "demo_data.h"
#include "demo.h"

namespace demo {

/// Helper Functions ///

float64_t ambient( const float64_t pTime ) {
    const static float64_t csLoTone = llce::sfx::freq( 'c', 0, 4 );
    const static float64_t csHiTone = llce::sfx::freq( 'c', 0, 5 );

    const float64_t cMixFactor = std::fmod( demo::COLOR_VELOCITY * pTime, 1.0 );
    const float64_t cCurrTone = glm::mix( csLoTone, csHiTone, cMixFactor );

    return llce::sfx::waveform::sine( cMixFactor, cCurrTone, 1.0, 0.0 );
}

/// Interface Functions ///

extern "C" bool32_t boot( demo::output_t* pOutput ) {
    // Initialize Graphics //

    const vec2u32_t cGFXBuffRes( 512, 512 );

    llce::output::boot<1, 1>( *pOutput, cGFXBuffRes );

    // Initialize Sound //

    // ... //

    return true;
}


extern "C" bool32_t init( demo::state_t* pState, demo::input_t* pInput ) {
    // Initialize Global Variables //

    pState->tt = 0.0f;

    pState->synth = llce::sfx::synth_t( &demo::VOLUME );
    pState->synth.play( ambient, std::numeric_limits<float64_t>::infinity() );

    // Initialize Input //

    std::memset( pInput, 0, sizeof(demo::input_t) );

    // Initialize Per-Mode Variables //

    return true;
}


extern "C" bool32_t update( demo::state_t* pState, demo::input_t* pInput, const demo::output_t* pOutput, const float64_t pDT ) {
    pState->tt += pDT;

    bool32_t updateStatus = true;
    updateStatus &= pState->synth.update( pDT, pOutput->sfxBufferFrames[llce::output::BUFFER_SHARED_ID] );
    return updateStatus;
}


extern "C" bool32_t render( const demo::state_t* pState, const demo::input_t* pInput, const demo::output_t* pOutput ) {
    llce::gfx::fbo_context_t metaFBOC(
        pOutput->gfxBufferFBOs[llce::output::BUFFER_SHARED_ID],
        pOutput->gfxBufferRess[llce::output::BUFFER_SHARED_ID] );

    // NOTE(JRC): The hue is normalized from the standard [0.0, 360.0) range
    // to simplify conversion calculations.
    color4f32_t hsvColor(
        std::fmod(demo::COLOR_VELOCITY * pState->tt, 1.0f),
        demo::COLOR_SATURATION, demo::COLOR_VALUE, 1.0f );
    color4f32_t rgbColor = llce::gfx::color::hsv2rgb( hsvColor );
    color4u8_t rgbColorByte = llce::gfx::color::f322u8( rgbColor );
    llce::gfx::render_context_t metaRC(
        llce::box_t(-1.0f, -1.0f, 2.0f, 2.0f),
        &rgbColorByte );
    metaRC.render();

    bool32_t renderStatus = true;
    renderStatus &= pState->synth.render( pOutput->sfxConfig, pOutput->sfxBuffers[llce::output::BUFFER_SHARED_ID] );
    return renderStatus;
}

}
