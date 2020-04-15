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

template <bool8_t Keyboard, bool8_t Mouse> struct input_t {};
template <> struct input_t<true, false> { keyboard_t keyboard; };
template <> struct input_t<false, true> { mouse_t mouse; };
template <> struct input_t<true, true> { keyboard_t keyboard; mouse_t mouse; };

/// Namespace Functions ///

bool32_t readKeyboard( keyboard_t& pKeyboard );
bool32_t readMouse( mouse_t& pMouse );

// TODO(JRC): Extend/combine these functions so that they work with all kinds
// of buttons (e.g. including mouse buttons, gamepad buttons, etc.).
bool32_t isKeyDown( const keyboard_t& pKeyboard, const SDL_Scancode pKey );
uint32_t isKGDown( const keyboard_t& pKeyboard, const SDL_Scancode* pKeyGroup, const uint32_t pGroupSize );

bool32_t isKeyPressed( const keyboard_t& pKeyboard, const SDL_Scancode pKey );
uint32_t isKGPressed( const keyboard_t& pKeyboard, const SDL_Scancode* pKeyGroup, const uint32_t pGroupSize );

bool32_t isKeyReleased( const keyboard_t& pKeyboard, const SDL_Scancode pKey );
uint32_t isKGReleased( const keyboard_t& pKeyboard, const SDL_Scancode* pKeyGroup, const uint32_t pGroupSize );

}

}

#endif
