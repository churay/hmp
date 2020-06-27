#ifndef META_LIB_H
#define META_LIB_H

#include <SDL2/SDL.h>

#include "deque.hpp"

#include "input.h"
#include "output.h"
#include "consts.h"

namespace meta {

/// State Types/Variables ///

LLCE_ENUM( mode, fps, audio );

constexpr static uint32_t FPS_FRAME_COUNT = 2 * LLCE_FPS;
constexpr static uint32_t AUDIO_SAMPLE_COUNT = 5 * LLCE_SPS * LLCE_MAX_CHANNELS;

struct state_t {
    mode_e mode;

    // FPS State //
    llce::deque<float64_t, meta::FPS_FRAME_COUNT> frameDTs;

    // Audio State //
    llce::deque<int16_t, meta::AUDIO_SAMPLE_COUNT> audioSamples;
};

/// Input/Output Types/Variables ///

typedef llce::input::input_t input_t;
typedef llce::output::output_t<1, 1> output_t;

/// Functions ///

bool32_t init( meta::state_t* pState, meta::input_t* pInput );
bool32_t boot( meta::output_t* pOutput );
bool32_t update( meta::state_t* pState, meta::input_t* pInput, const meta::output_t* pOutput, const float64_t pDT );
bool32_t render( const meta::state_t* pState, const meta::input_t* pInput, const meta::output_t* pOutput );

}


#endif
