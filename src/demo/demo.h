#ifndef DEMO_LIB_H
#define DEMO_LIB_H

#include <SDL2/SDL.h>

#include "sfx.h"
#include "input.h"
#include "output.h"

#include "consts.h"

namespace demo {

/// State Types/Variables ///

struct state_t {
    float32_t tt;
    llce::sfx::synth_t synth;
};

/// Input/Output Types/Variables ///

typedef llce::input::input_t<true, false> input_t;
typedef llce::output::output_t<1, 1> output_t;

}

/// Functions ///

#if !LLCE_DYLOAD
extern "C" {
    bool32_t init( demo::state_t* pState, demo::input_t* pInput );
    bool32_t boot( demo::output_t* pOutput );
    bool32_t update( demo::state_t* pState, demo::input_t* pInput, const demo::output_t* pOutput, const float64_t pDT );
    bool32_t render( const demo::state_t* pState, const demo::input_t* pInput, const demo::output_t* pOutput );
};
#endif

#endif
