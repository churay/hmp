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

#include "input.h"
#include "output.h"
#include "gfx.h"
#include "sfx.h"

#include "hmp_modes.h"
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

    const vec2u32_t cGFXBuffRes( 512, 512 );
    const std::array<llce::box_t, 3> cGFXBuffBoxs{{
        llce::box_t(0.0f, 0.0f, 1.0f, 1.0f),
        llce::box_t(0.0f, 0.0f, 1.0f, 0.85f),
        llce::box_t(0.0f, 0.85f, 1.0f, 0.15f)
    }};

    llce::output::boot<3, 1>( *pOutput, cGFXBuffRes, cGFXBuffBoxs );

    // Initialize Sound //

    // ... //

    return true;
}


extern "C" bool32_t init( hmp::state_t* pState, hmp::input_t* pInput ) {
    // Initialize Global Variables //

    pState->dt = 0.0;
    pState->tt = 0.0;

    pState->mid = hmp::mode::boot_id;
    pState->pmid = hmp::mode::menu_id;

    pState->rng = llce::rng_t( hmp::RNG_SEED );
    pState->synth = llce::sfx::synth_t();

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
    pState->synth.update( pDT, pOutput->sfxBufferFrames[llce::output::BUFFER_SHARED_ID] );
    return updateStatus;
}


extern "C" bool32_t render( const hmp::state_t* pState, const hmp::input_t* pInput, const hmp::output_t* pOutput ) {
    llce::gfx::render_context_t hmpRC(
        llce::box_t(-1.0f, -1.0f, 2.0f, 2.0f),
        &hmp::color::BACKGROUND );
    hmpRC.render();

    bool32_t renderStatus = MODE_RENDER_FUNS[pState->mid]( pState, pInput, pOutput );
    pState->synth.render( pOutput->sfxConfig, pOutput->sfxBuffers[llce::output::BUFFER_SHARED_ID] );
    return renderStatus;
}
