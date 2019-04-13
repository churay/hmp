#ifndef HMP_LIB_H
#define HMP_LIB_H

#include <SDL2/SDL.h>

#include "hmp_entities.h"
#include "hmp_consts.h"
#include "consts.h"

namespace hmp {

/// State Variables ///

constexpr static uint32_t MAX_ENTITIES = 16;
constexpr static float32_t ROUND_START_TIME = 1.0f;

/// Render Variables ///

constexpr static uint32_t SIM_RESOLUTION[2] = { 256, 256 };
constexpr static uint32_t UI_RESOLUTION[2] = { 256, 32 };

constexpr static color_t COLOR_WEST = { 0x9a, 0x86, 0x00, 0xFF };
constexpr static color_t COLOR_EAST = { 0x00, 0x9d, 0xa3, 0xFF };

/// Data Structures ///

struct state_t {
    float64_t dt, rt, tt;
    bool32_t roundStarted;

    // TODO(JRC): The following variables will work as state variables since
    // the application always uses the same values for these buffers, but this
    // assumption may not always hold going forward.
    uint32_t simBufferGLID, uiBufferGLID;
    uint32_t simTextureGLID, uiTextureGLID;

    entity_t* entities[hmp::MAX_ENTITIES];
    bounds_t boundsEnt;
    bounds_t ricochetEnts[2];
    scoreboard_t scoreEnt;
    ball_t ballEnt;
    paddle_t paddleEnts[2];
};

/// Input Types/Variables ///

constexpr static uint8_t KEY_DIFF_NONE = 0;
constexpr static uint8_t KEY_DIFF_DOWN = 1;
constexpr static uint8_t KEY_DIFF_UP = 2;

struct input_t {
    uint8_t keys[SDL_Scancode::SDL_NUM_SCANCODES];
    uint8_t diffs[SDL_Scancode::SDL_NUM_SCANCODES];
};

}

#endif
