#ifndef LLCE_INPUT_H
#define LLCE_INPUT_H

#include <SDL2/SDL.h>

#include "consts.h"

namespace llce {

namespace input {

/// Namespace Attributes ///

enum class inputdiff_e : uint8_t { none = 0, down = 1, up = 2 };

typedef uint8_t keystate_t[SDL_Scancode::SDL_NUM_SCANCODES];
typedef inputdiff_e keydiffs_t[SDL_Scancode::SDL_NUM_SCANCODES];
typedef struct keyboard { keystate_t keys = {}; keydiffs_t diffs = {}; } keyboard_t;

// TODO(JRC): Improve this implementation if SDL2 ever offsets a better variable
// that tracks the number of supported mouse buttons.
static constexpr uint32_t SDL_NUM_MOUSECODES = SDL_BUTTON_X2 + 1;
typedef uint8_t mousestate_t[SDL_NUM_MOUSECODES];
typedef inputdiff_e mousediffs_t[SDL_NUM_MOUSECODES];
typedef struct mouse { vec2i32_t global; vec2i32_t window; mousestate_t buttons = {}; mousediffs_t diffs = {}; } mouse_t;

/// Namespace Types ///

// NOTE(JRC): Using templates allows for individual simulations to configure the
// exact set of input devices they read, which helps to cut down on each simulation's
// memory footprint.
template <bool8_t Keyboard, bool8_t Mouse>
struct input_t {
    constexpr static uint32_t HAS_KEYBOARD = Keyboard;
    constexpr static uint32_t HAS_MOUSE = Mouse;

    // NOTE(JRC): There are a lot of places in the 'llce' code where 'memset' is
    // used to move input data from different buffers (e.g. the replay buffer,
    // the harness buffer, the simulation buffer, etc.), so these accessors to the
    // underlying data must be functions and not raw pointers.
    inline keyboard_t* keyboard() { return HAS_KEYBOARD ? &_keyboard[0] : nullptr; }
    inline mouse_t* mouse() { return HAS_MOUSE ? &_mouse[0] : nullptr; }

    keyboard_t _keyboard[Keyboard];
    mouse_t _mouse[Mouse];
};

/// Namespace Functions ///

bool32_t readKeyboard( keyboard_t* pKeyboard );
bool32_t readMouse( mouse_t* pMouse );

template <bool8_t Keyboard, bool8_t Mouse>
bool32_t readInput( input_t<Keyboard, Mouse>* pInput ) {
    return
        ( Keyboard ? llce::input::readKeyboard(pInput->keyboard()) : true ) &&
        ( Mouse ? llce::input::readMouse(pInput->mouse()) : true );
}

// TODO(JRC): Extend/combine these functions so that they work with all kinds
// of buttons (e.g. including mouse buttons, gamepad buttons, etc.).
bool32_t isKeyDown( const keyboard_t* pKeyboard, const SDL_Scancode pKey );
uint32_t isKGDown( const keyboard_t* pKeyboard, const SDL_Scancode* pKeyGroup, const uint32_t pGroupSize );

bool32_t isKeyPressed( const keyboard_t* pKeyboard, const SDL_Scancode pKey );
uint32_t isKGPressed( const keyboard_t* pKeyboard, const SDL_Scancode* pKeyGroup, const uint32_t pGroupSize );

bool32_t isKeyReleased( const keyboard_t* pKeyboard, const SDL_Scancode pKey );
uint32_t isKGReleased( const keyboard_t* pKeyboard, const SDL_Scancode* pKeyGroup, const uint32_t pGroupSize );

}

}

#endif
