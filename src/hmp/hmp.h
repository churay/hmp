#ifndef HMP_LIB_H
#define HMP_LIB_H

#include <SDL2/SDL.h>

#include "hmp_entities.h"
#include "hmp_consts.h"

#include "rng_t.h"
#include "gui.h"
#include "input.h"
#include "output.h"
#include "gfx.h"
#include "sfx.h"
#include "consts.h"

namespace hmp {

/// State Types/Variables ///

constexpr static float32_t ROUND_START_TIME = 1.0f;

struct state_t {
    // Global State //
    float64_t dt; // frame time
    float64_t tt; // total time

    mode::mode_e mid; // game mode
    mode::mode_e pmid; // pending mode

    llce::rng_t rng; // random number generator
    llce::sfx::synth_t synth; // sound effect synthesizer

    // Game State //
    float64_t rt; // round time
    bool32_t roundStarted; // round flag
    bool32_t roundPaused; // round pause flag
    team::team_e roundServer; // round starter flag

    bounds_t boundsEnt;
    bounds_t ricochetEnts[2];
    scoreboard_t scoreEnt;
    ball_t ballEnt;
    paddle_t paddleEnts[2];

    // Menu State //
    llce::gui::menu_t titleMenu;
    llce::gui::menu_t resetMenu;
};

/// Input/Output Types/Variables ///

constexpr static uint32_t GFX_BUFFER_MASTER_ID = llce::output::BUFFER_SHARED_ID;
constexpr static uint32_t GFX_BUFFER_SIM_ID = GFX_BUFFER_MASTER_ID + 1;
constexpr static uint32_t GFX_BUFFER_UI_ID = GFX_BUFFER_SIM_ID + 1;
constexpr static uint32_t GFX_BUFFER_COUNT = GFX_BUFFER_UI_ID + 1;

constexpr static uint32_t SFX_BUFFER_COUNT = llce::output::BUFFER_SHARED_ID + 1;

typedef llce::input::input_t<true, false> input_t;
typedef llce::output::output_t<GFX_BUFFER_COUNT, SFX_BUFFER_COUNT> output_t;

}

#if !LLCE_DYLOAD
extern "C" {
    bool32_t init( hmp::state_t* pState, hmp::input_t* pInput );
    bool32_t boot( hmp::output_t* pOutput );
    bool32_t update( hmp::state_t* pState, hmp::input_t* pInput, const hmp::output_t* pOutput, const float64_t pDT );
    bool32_t render( const hmp::state_t* pState, const hmp::input_t* pInput, const hmp::output_t* pOutput );
};
#endif

#endif
