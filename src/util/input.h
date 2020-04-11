#ifndef LLCE_INPUT_H
#define LLCE_INPUT_H

#include <SDL2/SDL.h>

#include "consts.h"

namespace llce {

namespace input {

/// Namespace Attributes ///

enum class keydiff_e : uint8_t { none = 0, down = 1, up = 2 };
typedef uint8_t keystate_t[SDL_Scancode::SDL_NUM_SCANCODES];
typedef keydiff_e keydiffs_t[SDL_Scancode::SDL_NUM_SCANCODES];
typedef struct keyboard { keystate_t keys = {}; keydiffs_t diffs = {}; } keyboard_t;
typedef struct mouse { SDL_Window* focus = NULL; vec2i32_t global; vec2i32_t window; } mouse_t;

template <bool8_t Keyboard, bool8_t Mouse> struct input_t {};
template <> struct input_t<true, false> { keyboard_t keyboard; };
template <> struct input_t<false, true> { mouse_t mouse; };
template <> struct input_t<true, true> { keyboard_t keyboard; mouse_t mouse; };

/// Namespace Functions ///

bool32_t readKeyboard( keyboard_t& pKeyboard );

bool32_t isKeyDown( const keyboard_t& pKeyboard, const SDL_Scancode pKey );
uint32_t isKGDown( const keyboard_t& pKeyboard, const SDL_Scancode* pKeyGroup, const uint32_t pGroupSize );

bool32_t isKeyPressed( const keyboard_t& pKeyboard, const SDL_Scancode pKey );
uint32_t isKGPressed( const keyboard_t& pKeyboard, const SDL_Scancode* pKeyGroup, const uint32_t pGroupSize );

bool32_t isKeyReleased( const keyboard_t& pKeyboard, const SDL_Scancode pKey );
uint32_t isKGReleased( const keyboard_t& pKeyboard, const SDL_Scancode* pKeyGroup, const uint32_t pGroupSize );

}

}

#endif
