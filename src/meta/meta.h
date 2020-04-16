#ifndef META_LIB_H
#define META_LIB_H

#include <SDL2/SDL.h>

#include "deque.hpp"

#include "input.h"
#include "output.h"
#include "consts.h"

namespace meta {

/// State Types/Variables ///

constexpr static uint32_t UI_FPS_FRAME_COUNT = 2 * LLCE_FPS;
constexpr static color4u8_t UI_FPS_BACKGROUND_COLOR = { 0xFF, 0xFF, 0xFF, 0xFF };

struct state_t {
    // FPS State //
    llce::deque<float64_t, meta::UI_FPS_FRAME_COUNT> dts;
};

/// Input/Output Types/Variables ///

typedef llce::input::input_t input_t;
typedef llce::output::output_t<1, 0> output_t;

/// Functions ///

bool32_t init( meta::state_t* pState, meta::input_t* pInput );
bool32_t boot( meta::output_t* pOutput );
bool32_t update( meta::state_t* pState, meta::input_t* pInput, const meta::output_t* pOutput, const float64_t pDT );
bool32_t render( const meta::state_t* pState, const meta::input_t* pInput, const meta::output_t* pOutput );

}


#endif
