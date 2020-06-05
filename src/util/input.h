#ifndef LLCE_INPUT_H
#define LLCE_INPUT_H

#include <SDL2/SDL.h>

#include "consts.h"

namespace llce {

namespace input {

/// Namespace Attributes ///

enum class device_e : uint8_t { keyboard = 0, mouse = 1 }; // gamepad = 2, gyro = 3, ... };
enum class type_e : uint8_t { d0 = 0, d1 = 1, d2 = 2 };
enum class diff_e : uint8_t { none = 0, down = 1, up = 2 };

static constexpr uint32_t SDL_NUM_KEYCODES = SDL_Scancode::SDL_NUM_SCANCODES;
typedef uint8_t keystate_t[SDL_NUM_KEYCODES];
typedef diff_e keydiffs_t[SDL_NUM_KEYCODES];
typedef struct keyboard { keystate_t keys = {}; keydiffs_t diffs = {}; } keyboard_t;

static constexpr uint32_t SDL_NUM_MOUSECODES = SDL_BUTTON_X2 + 1;
typedef uint8_t mousestate_t[SDL_NUM_MOUSECODES];
typedef diff_e mousediffs_t[SDL_NUM_MOUSECODES];
typedef struct mouse { vec2i32_t global; vec2i32_t window; mousestate_t buttons = {}; mousediffs_t diffs = {}; } mouse_t;

// TODO(JRC): This should be improved so that the position of the mouse is included
// as an input stream as well.
static constexpr uint32_t SDL_NUM_DEVCODES[] = { SDL_NUM_KEYCODES, SDL_NUM_MOUSECODES };
static constexpr uint32_t SDL_NUM_INPUTS = SDL_NUM_DEVCODES[0] + SDL_NUM_DEVCODES[1];

constexpr static uint32_t MAX_ACTION_BINDINGS = 4;
constexpr static uint8_t ACTION_UNBOUND_ID = 0;

/// Namespace Types ///

struct stream_t {
    stream_t( uint32_t pID, device_e pDevID );
    stream_t( uint32_t pID, uint32_t pDevID );
    stream_t( uint64_t pGlobalID );

    uint32_t gid() const;
    uint64_t did() const;

    operator uint32_t() const;
    operator uint64_t() const;

    uint32_t mID;
    device_e mDevID;
};


struct binding_t {
    binding_t();
    binding_t( const uint32_t* pActionIDs, const uint32_t* pGlobalInputIDs, const uint32_t pInputSize );

    bool32_t bind( const uint32_t pActionID, const uint32_t* pGlobalInputIDs, const uint32_t pGlobalInputSize );
    uint32_t* find( const uint32_t pActionID );

    // sim-specific action id => [bound sim-agnostic input global id]
    uint32_t mActionBindings[LLCE_MAX_ACTIONS][MAX_ACTION_BINDINGS];
    // sim-agnostic global id => sim-specific action id
    uint32_t mBoundActions[SDL_NUM_INPUTS + 1];
};


// NOTE(JRC): There are a lot of places in the 'llce' code where 'memset' is
// used to move input data from different buffers (e.g. the replay buffer,
// the harness buffer, the simulation buffer, etc.), so these accessors to the
// underlying data must be functions and not raw pointers.
template <bool8_t Keyboard, bool8_t Mouse>
struct input_t {
    binding_t binding;
    keyboard_t _keyboard[Keyboard];
    mouse_t _mouse[Mouse];

    inline keyboard_t* keyboard() { return Keyboard ? &_keyboard[0] : nullptr; }
    inline const keyboard_t* keyboard() const { return Keyboard ? &_keyboard[0] : nullptr; }

    inline mouse_t* mouse() { return Mouse ? &_mouse[0] : nullptr; }
    inline const mouse_t* mouse() const { return Mouse ? &_mouse[0] : nullptr; }
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
// TODO(JRC): A style of combined input system (just 0D to start) needs to be implemented
// using the global IDs codified into the 'bindings_t' class.
bool32_t isKeyDown( const keyboard_t* pKeyboard, const SDL_Scancode pKey );
uint32_t isKGDown( const keyboard_t* pKeyboard, const SDL_Scancode* pKeyGroup, const uint32_t pGroupSize );

bool32_t isKeyPressed( const keyboard_t* pKeyboard, const SDL_Scancode pKey );
uint32_t isKGPressed( const keyboard_t* pKeyboard, const SDL_Scancode* pKeyGroup, const uint32_t pGroupSize );

bool32_t isKeyReleased( const keyboard_t* pKeyboard, const SDL_Scancode pKey );
uint32_t isKGReleased( const keyboard_t* pKeyboard, const SDL_Scancode* pKeyGroup, const uint32_t pGroupSize );

}

}

#endif
