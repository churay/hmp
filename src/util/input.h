#ifndef LLCE_INPUT_H
#define LLCE_INPUT_H

#include <SDL2/SDL.h>

#include "consts.h"

namespace llce {

namespace input {

/// Namespace Attributes ///

LLCE_ENUM( device, unbound, keyboard, mouse ); // gamepad, gyro );
enum class diff_e : uint8_t { none = 0, down = 1, up = 2 };

constexpr static uint32_t ACTION_UNBOUND_ID = 0;
constexpr static uint32_t INPUT_UNBOUND_ID = 0;

struct input_t;
typedef bool32_t (*diff_f)( const input_t* pInput, const uint32_t pInputGID );

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
static constexpr uint32_t SDL_NUM_INPUTS = 1 + SDL_NUM_KEYCODES + SDL_NUM_MOUSECODES;

bool32_t isDown( const input_t* pInput, const uint32_t pInputGID );
bool32_t isPressed( const input_t* pInput, const uint32_t pInputGID );
bool32_t isReleased( const input_t* pInput, const uint32_t pInputGID );

/// Namespace Types ///

struct stream_t {
    stream_t();
    stream_t( device_e pDevID, uint32_t pID );
    stream_t( uint32_t pDevID, uint32_t pID );
    stream_t( uint32_t pGlobalID );
    stream_t( uint64_t pDeviceID );

    uint32_t gid() const;
    uint64_t did() const;
    const char8_t* name() const;

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

// TODO(JRC): If so desired, the nice feature of only allocating memory to
// relevant/supported input streams can be implemented by having pre-processor
// CMake variables associated with the application (e.g. LLCE_HAS_KEYBOARD)
// and then applying these preprocessor directives as appropriate.
// TODO(JRC): Given that 'input_t' is going to become more accessible, the
// next logical step is to wonder how to integrate 'binding_t' for tighter
// data coupling. Is there any good way (e.g. via #defines) to provide a pointer
// to the start of the 'input_t', and then the size, depending on user configuration?
struct input_t {
    keyboard_t mKeyboard;
    mouse_t mMouse;
    binding_t mBinding;

    // declare size as static so that it can be used by the harness when doing 'memcpy' operations

    bool32_t read( const device_e pDevID = llce::input::device::unbound );

    uint8_t* state( const device_e pDevID );
    const uint8_t* state( const device_e pDevID ) const;

    diff_e* diffs( const device_e pDevID );
    const diff_e* diffs( const device_e pDevID ) const;

    bool32_t isDiffRaw( diff_f pDiff, const uint32_t pInputGID ) const;
    uint32_t isDiffRaw( diff_f pDiff, const uint32_t* pInputGIDs ) const;
    uint32_t isDiffAct( diff_f pDiff, const uint32_t pInputAction ) const;
    uint32_t isDiffAct( diff_f pDiff, const uint32_t* pInputActions ) const;

    inline bool32_t isDownRaw( const uint32_t pInputGID ) const { return isDiffRaw(isDown, pInputGID); }
    inline uint32_t isDownRaw( const uint32_t* pInputGIDs ) const { return isDiffRaw(isDown, pInputGIDs); }
    inline uint32_t isDownAct( const uint32_t pInputAction ) const { return isDiffAct(isDown, pInputAction); }
    inline uint32_t isDownAct( const uint32_t* pInputActions ) const { return isDiffAct(isDown, pInputActions); }

    inline bool32_t isPressedRaw( const uint32_t pInputGID ) const { return isDiffRaw(isPressed, pInputGID); }
    inline uint32_t isPressedRaw( const uint32_t* pInputGIDs ) const { return isDiffRaw(isPressed, pInputGIDs); }
    inline uint32_t isPressedAct( const uint32_t pInputAction ) const { return isDiffAct(isPressed, pInputAction); }
    inline uint32_t isPressedAct( const uint32_t* pInputActions ) const { return isDiffAct(isPressed, pInputActions); }

    inline bool32_t isReleasedRaw( const uint32_t pInputGID ) const { return isDiffRaw(isReleased, pInputGID); }
    inline uint32_t isReleasedRaw( const uint32_t* pInputGIDs ) const { return isDiffRaw(isReleased, pInputGIDs); }
    inline uint32_t isReleasedAct( const uint32_t pInputAction ) const { return isDiffAct(isReleased, pInputAction); }
    inline uint32_t isReleasedAct( const uint32_t* pInputActions ) const { return isDiffAct(isReleased, pInputActions); }
};

/// Namespace Functions ///

// bool32_t isDown( const uint32_t* pState, const diff_e* pDiffs, const uint32_t pInputGID );
// bool32_t isPressed( const uint32_t* pState, const diff_e* pDiffs, const uint32_t pInputGID );
// bool32_t isReleased( const uint32_t* pState, const diff_e* pDiffs, const uint32_t pInputGID );

const char8_t* identify( const uint32_t pInputGID );
bool32_t identify( const uint32_t pInputGID, char8_t* pBuffer, const uint32_t pBufferLength );

}

}

#endif
