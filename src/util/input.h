#ifndef LLCE_INPUT_H
#define LLCE_INPUT_H

#include <SDL2/SDL.h>

#include "consts.h"

namespace llce {

namespace input {

/// Namespace Attributes ///

enum class device_e : uint8_t { unbound = 0, keyboard = 1, mouse = 2 }; // gamepad = 3, gyro = 4, ... };
enum class type_e : uint8_t { d0 = 0, d1 = 1, d2 = 2 };
enum class diff_e : uint8_t { none = 0, down = 1, up = 2 };

constexpr static uint32_t ACTION_UNBOUND_ID = 0;
constexpr static uint32_t INPUT_UNBOUND_ID = 0;

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
static constexpr uint32_t SDL_NUM_DEVCODES[] = { 1, SDL_NUM_KEYCODES, SDL_NUM_MOUSECODES };
static constexpr uint32_t SDL_NUM_INPUTS = 1 + SDL_NUM_DEVCODES[0] + SDL_NUM_DEVCODES[1];

/// Namespace Types ///

struct stream_t {
    stream_t();
    stream_t( device_e pDevID, uint32_t pID );
    stream_t( uint32_t pDevID, uint32_t pID );
    stream_t( uint32_t pGlobalID );
    stream_t( uint64_t pDeviceID );

    uint32_t gid() const;
    uint64_t did() const;

    operator uint32_t() const;
    operator uint64_t() const;

    device_e mDevID;
    uint32_t mID;
};


// TODO(JRC): The GID functions can potentially be changed to 'stream_t' functions
// and use C++ replace-in-place substitution to cut down on typing.
// TODO(JRC): Experiment with dictionary-esque interface (e.g. for bind/find;
// read/write) to see if it improves usability/terseness.
struct binding_t {
    binding_t();
    binding_t( const uint32_t* pInputGIDs );

    bool32_t bind( const uint32_t pActionID, const uint32_t* pInputGIDs );
    uint32_t* find( const uint32_t pActionID );
    const uint32_t* find( const uint32_t pActionID ) const;

    // sim-specific action id => [bound sim-agnostic input global id]
    uint32_t mActionBindings[LLCE_MAX_ACTIONS][LLCE_MAX_BINDINGS + 1];
    // sim-agnostic global id => sim-specific action id
    uint32_t mBoundActions[SDL_NUM_INPUTS];
};


template <bool8_t Keyboard, bool8_t Mouse>
struct input_t {
    keyboard_t _keyboard[Keyboard];
    mouse_t _mouse[Mouse];

    // NOTE(JRC): There are a lot of places in the 'llce' code where 'memset' is
    // used to move input data from different buffers (e.g. the replay buffer,
    // the harness buffer, the simulation buffer, etc.), so these accessors to the
    // underlying data must be functions and not raw pointers.
    inline keyboard_t* keyboard() { return Keyboard ? &_keyboard[0] : nullptr; }
    inline const keyboard_t* keyboard() const { return Keyboard ? &_keyboard[0] : nullptr; }

    inline mouse_t* mouse() { return Mouse ? &_mouse[0] : nullptr; }
    inline const mouse_t* mouse() const { return Mouse ? &_mouse[0] : nullptr; }

    // TODO(JRC): Differentiate these 0D streams (which will return a list of
    // bools) from other stream types (e.g. 2D streams, which will likely use floats).
    inline uint8_t* state( device_e pDevID ) {
        return (
            (Keyboard && pDevID == device_e::keyboard) ? &_keyboard[0].keys[0] : (
            (Mouse && pDevID == device_e::mouse) ? &_mouse[0].buttons[0] : nullptr ));
    }
    inline const uint8_t* state( device_e pDevID ) const {
        return (
            (Keyboard && pDevID == device_e::keyboard) ? &_keyboard[0].keys[0] : (
            (Mouse && pDevID == device_e::mouse) ? &_mouse[0].buttons[0] : nullptr ));
    }

    inline diff_e* diffs( device_e pDevID ) {
        return (
            (Keyboard && pDevID == device_e::keyboard) ? &_keyboard[0].diffs[0] : (
            (Mouse && pDevID == device_e::mouse) ? &_mouse[0].diffs[0] : nullptr ));
    }
    inline const diff_e* diffs( device_e pDevID ) const {
        return (
            (Keyboard && pDevID == device_e::keyboard) ? &_keyboard[0].diffs[0] : (
            (Mouse && pDevID == device_e::mouse) ? &_mouse[0].diffs[0] : nullptr ));
    }
};

/// Namespace Functions ///

bool32_t readKeyboard( keyboard_t* pKeyboard );
bool32_t readMouse( mouse_t* pMouse );

bool32_t isKeyDown( const keyboard_t* pKeyboard, const SDL_Scancode pKey );
uint32_t isKGDown( const keyboard_t* pKeyboard, const SDL_Scancode* pKeyGroup, const uint32_t pGroupSize );

bool32_t isKeyPressed( const keyboard_t* pKeyboard, const SDL_Scancode pKey );
uint32_t isKGPressed( const keyboard_t* pKeyboard, const SDL_Scancode* pKeyGroup, const uint32_t pGroupSize );

bool32_t isKeyReleased( const keyboard_t* pKeyboard, const SDL_Scancode pKey );
uint32_t isKGReleased( const keyboard_t* pKeyboard, const SDL_Scancode* pKeyGroup, const uint32_t pGroupSize );


template <bool8_t Keyboard, bool8_t Mouse>
bool32_t readInput( input_t<Keyboard, Mouse>* pInput ) {
    return
        ( Keyboard ? llce::input::readKeyboard(pInput->keyboard()) : true ) &&
        ( Mouse ? llce::input::readMouse(pInput->mouse()) : true );
}


template <bool8_t Keyboard, bool8_t Mouse>
bool32_t isDown( const input_t<Keyboard, Mouse>* pInput, const uint32_t pInputGID ) {
    const stream_t cInputStream( pInputGID );
    const uint8_t* cInputState = pInput->state( cInputStream.mDevID );
    return (bool32_t)( cInputState != nullptr && cInputState[cInputStream.mID] );
}

template <bool8_t Keyboard, bool8_t Mouse>
uint32_t isDown( const input_t<Keyboard, Mouse>* pInput, const uint32_t* pInputGIDs ) {
    uint32_t firstIdx = INPUT_UNBOUND_ID;
    for( uint32_t inputIdx = 0; pInputGIDs[inputIdx] != INPUT_UNBOUND_ID && firstIdx == INPUT_UNBOUND_ID; inputIdx++ ) {
        firstIdx = isDown( pInput, pInputGIDs[inputIdx] ) ? pInputGIDs[inputIdx] : INPUT_UNBOUND_ID;
    }
    return firstIdx;
}

template <bool8_t Keyboard, bool8_t Mouse>
uint32_t isDown( const input_t<Keyboard, Mouse>* pInput, const binding_t* pBinding, const uint32_t* pInputActions ) {
    uint32_t firstAction = ACTION_UNBOUND_ID;
    for( uint32_t actionIdx = 0; pInputActions[actionIdx] != ACTION_UNBOUND_ID && firstAction == ACTION_UNBOUND_ID; actionIdx++ ) {
        const uint32_t* cActionBindings = pBinding->find( pInputActions[actionIdx] );
        firstAction = isDown( pInput, cActionBindings ) ? pInputActions[actionIdx] : ACTION_UNBOUND_ID;
    }
    return firstAction;
}


template <bool8_t Keyboard, bool8_t Mouse>
bool32_t isPressed( const input_t<Keyboard, Mouse>* pInput, const uint32_t pInputGID ) {
    const stream_t cInputStream( pInputGID );
    const diff_e* cInputDiffs = pInput->diffs( cInputStream.mDevID );
    return (bool32_t)( cInputDiffs != nullptr && (cInputDiffs[cInputStream.mID] == diff_e::down) );
}

template <bool8_t Keyboard, bool8_t Mouse>
uint32_t isPressed( const input_t<Keyboard, Mouse>* pInput, const uint32_t* pInputGIDs ) {
    uint32_t firstIdx = INPUT_UNBOUND_ID;
    for( uint32_t inputIdx = 0; pInputGIDs[inputIdx] != INPUT_UNBOUND_ID && firstIdx == INPUT_UNBOUND_ID; inputIdx++ ) {
        firstIdx = isPressed( pInput, pInputGIDs[inputIdx] ) ? pInputGIDs[inputIdx] : INPUT_UNBOUND_ID;
    }
    return firstIdx;
}

template <bool8_t Keyboard, bool8_t Mouse>
uint32_t isPressed( const input_t<Keyboard, Mouse>* pInput, const binding_t* pBinding, const uint32_t* pInputActions ) {
    uint32_t firstAction = ACTION_UNBOUND_ID;
    for( uint32_t actionIdx = 0; pInputActions[actionIdx] != ACTION_UNBOUND_ID && firstAction == ACTION_UNBOUND_ID; actionIdx++ ) {
        const uint32_t* cActionBindings = pBinding->find( pInputActions[actionIdx] );
        firstAction = isPressed( pInput, cActionBindings ) ? pInputActions[actionIdx] : ACTION_UNBOUND_ID;
    }
    return firstAction;
}


template <bool8_t Keyboard, bool8_t Mouse>
bool32_t isReleased( const input_t<Keyboard, Mouse>* pInput, const uint32_t pInputGID ) {
    const stream_t cInputStream( pInputGID );
    const diff_e* cInputDiffs = pInput->diffs( cInputStream.mDevID );
    return (bool32_t)( cInputDiffs != nullptr && (cInputDiffs[cInputStream.mID] == diff_e::up) );
}

template <bool8_t Keyboard, bool8_t Mouse>
uint32_t isReleased( const input_t<Keyboard, Mouse>* pInput, const uint32_t* pInputGIDs ) {
    uint32_t firstIdx = INPUT_UNBOUND_ID;
    for( uint32_t inputIdx = 0; pInputGIDs[inputIdx] != INPUT_UNBOUND_ID && firstIdx == INPUT_UNBOUND_ID; inputIdx++ ) {
        firstIdx = isReleased( pInput, pInputGIDs[inputIdx] ) ? pInputGIDs[inputIdx] : INPUT_UNBOUND_ID;
    }
    return firstIdx;
}

template <bool8_t Keyboard, bool8_t Mouse>
uint32_t isReleased( const input_t<Keyboard, Mouse>* pInput, const binding_t* pBinding, const uint32_t* pInputActions ) {
    uint32_t firstAction = ACTION_UNBOUND_ID;
    for( uint32_t actionIdx = 0; pInputActions[actionIdx] != ACTION_UNBOUND_ID && firstAction == ACTION_UNBOUND_ID; actionIdx++ ) {
        const uint32_t* cActionBindings = pBinding->find( pInputActions[actionIdx] );
        firstAction = isReleased( pInput, cActionBindings ) ? pInputActions[actionIdx] : ACTION_UNBOUND_ID;
    }
    return firstAction;
}

}

}

#endif
