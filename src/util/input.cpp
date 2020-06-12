#include <SDL2/SDL.h>

#include <cmath>
#include <cstring>

#include "input.h"

namespace llce {

namespace input {

/// 'llce::input::stream_t' Functions ///

stream_t::stream_t() :
        mDevID( device_e::unbound ), mID( 0 ) {
    
}


stream_t::stream_t( device_e pDevID, uint32_t pID ) :
        mDevID( pDevID ), mID( pID ) {
    
}


stream_t::stream_t( uint32_t pDevID, uint32_t pID ) :
        mDevID( static_cast<device_e>(pDevID) ), mID( pID ) {
    
}


stream_t::stream_t( uint32_t pGlobalID ) {
    uint32_t deviceID = 0, deviceOffset = SDL_NUM_DEVCODES[0];
    while( deviceOffset < pGlobalID ) {
        deviceOffset += SDL_NUM_DEVCODES[++deviceID];
    }
    uint32_t inputID = pGlobalID - ( deviceOffset - SDL_NUM_DEVCODES[deviceID] );

    mDevID = static_cast<device_e>( deviceID );
    mID = inputID;
}


stream_t::stream_t( uint64_t pDeviceID ) :
        mDevID( static_cast<device_e>(pDeviceID >> 32) ), mID( pDeviceID >> 0 ) {
    
}


uint32_t stream_t::gid() const {
    return static_cast<uint32_t>(*this);
}


uint64_t stream_t::did() const {
    return static_cast<uint64_t>(*this);
}


const char8_t* stream_t::name() const {
    // TODO(JRC): Implement this function.
    // SDL_SCANCODE_TO_KEYCODE
    return nullptr;
}


stream_t::operator uint32_t() const {
    uint32_t deviceOffset = 0;
    for( uint32_t deviceID = 0; deviceID < (uint32_t)mDevID; deviceID++ ) {
        deviceOffset += SDL_NUM_DEVCODES[deviceID];
    }

    return deviceOffset + mID;
}


stream_t::operator uint64_t() const {
    uint64_t deviceID = static_cast<uint64_t>( mDevID ), localID = mID;
    return ( deviceID << 32 ) | ( localID << 0 );
}

/// 'llce::input::binding_t' Functions ///

binding_t::binding_t() {
    std::memset( &mActionBindings[0], INPUT_UNBOUND_ID, sizeof(mActionBindings) );
    std::memset( &mBoundActions[0], ACTION_UNBOUND_ID, sizeof(mBoundActions) );
}


// TODO(JRC): This signature limits default configurations to one binding per
// action. This should be extended to arbitrary bindings per action (provided
// such a change doesn't overcomplicate the prototype).
binding_t::binding_t( const uint32_t* pInputGIDs ) : binding_t() {
    for( uint32_t actionIdx = 1;
            actionIdx < LLCE_MAX_ACTIONS && pInputGIDs[actionIdx] != INPUT_UNBOUND_ID;
            actionIdx++ ) {
        mActionBindings[actionIdx][0] = pInputGIDs[actionIdx];
    }
}


bool32_t binding_t::bind( const uint32_t pActionID, const uint32_t* pInputGIDs ) {
    bool32_t validBind = true;

    LLCE_VERIFY_ERROR( validBind &= (0 < pActionID && pActionID < LLCE_MAX_ACTIONS),
        "Invalid action ID '" << pActionID << "'; "
        "valid range is [1, " << LLCE_MAX_ACTIONS << ")." );
    LLCE_VERIFY_ERROR( validBind &= (pInputGIDs[0] != INPUT_UNBOUND_ID),
        "Binding no inputs to an action; each bound action must have at least " <<
        "one corresponding input." );

    if( validBind ) {
        const static uint32_t csActionBytes = ( LLCE_MAX_BINDINGS + 1 ) * sizeof( uint32_t );
        std::memset( &mActionBindings[pActionID], INPUT_UNBOUND_ID, csActionBytes );

        for( uint32_t bindingIdx = 0;
                bindingIdx < LLCE_MAX_BINDINGS && pInputGIDs[bindingIdx] != INPUT_UNBOUND_ID;
                bindingIdx++ ) {
            mActionBindings[pActionID][bindingIdx] = pInputGIDs[bindingIdx];
        }
    }

    return validBind;
}


uint32_t* binding_t::find( const uint32_t pActionID ) {
    return &mActionBindings[pActionID][0];
}


const uint32_t* binding_t::find( const uint32_t pActionID ) const {
    return &mActionBindings[pActionID][0];
}

/// 'llce::input::input_t' Functions ///

bool32_t input_t::read( const device_e pDevID ) {
    bool32_t success = true;

    if( pDevID == device_e::unbound || pDevID == device_e::keyboard ) {
        const uint8_t* keyboardState = SDL_GetKeyboardState( nullptr );

        for( uint32_t keyIdx = 0; keyIdx < sizeof(keyboard.keys); keyIdx++ ) {
            const bool8_t wasKeyDown = keyboard.keys[keyIdx];
            const bool8_t isKeyDown = keyboardState[keyIdx];

            keyboard.keys[keyIdx] = isKeyDown;
            keyboard.diffs[keyIdx] = (
                (!wasKeyDown && isKeyDown) ? diff_e::down : (
                (wasKeyDown && !isKeyDown) ? diff_e::up : (
                diff_e::none)) );
        }

        success &= true;
    } if( pDevID == device_e::unbound || pDevID == device_e::mouse ) {
        // NOTE(JRC): The global mouse state will report the state of the mouse
        // regardless of where it's located on the screen where the window mouse
        // state will only report buttons pressed while the mouse is in focus.
        // This being the case, the window button mask is preferred as user input.
        const uint32_t cWindowButtonMask = SDL_GetMouseState( &mouse.window.x, &mouse.window.y );
        SDL_GetGlobalMouseState( &mouse.global.x, &mouse.global.y );

        for( uint32_t buttonIdx = 1; buttonIdx < sizeof(mouse.buttons); buttonIdx++ ) {
            const bool8_t wasButtonDown = mouse.buttons[buttonIdx];
            const bool8_t isButtonDown = cWindowButtonMask & SDL_BUTTON( buttonIdx );

            mouse.buttons[buttonIdx] = isButtonDown;
            mouse.diffs[buttonIdx] = (
                (!wasButtonDown && isButtonDown) ? diff_e::down : (
                (wasButtonDown && !isButtonDown) ? diff_e::up : (
                diff_e::none)) );
        }

        // TODO(JRC): Consider readding this field to the 'mouse_t' type should multi-
        // window simulations ever become supported.
        // mouse.focus = SDL_GetMouseFocus();
        success &= true;
    }

    return success;
}


uint8_t* input_t::state( const device_e pDevID ) {
    return (
        (pDevID == device_e::keyboard) ? &keyboard.keys[0] : (
        (pDevID == device_e::mouse) ? &mouse.buttons[0] : nullptr ));
}


const uint8_t* input_t::state( const device_e pDevID ) const {
    return (
        (pDevID == device_e::keyboard) ? &keyboard.keys[0] : (
        (pDevID == device_e::mouse) ? &mouse.buttons[0] : nullptr ));
}


diff_e* input_t::diffs( const device_e pDevID ) {
    return (
        (pDevID == device_e::keyboard) ? &keyboard.diffs[0] : (
        (pDevID == device_e::mouse) ? &mouse.diffs[0] : nullptr ));
}


const diff_e* input_t::diffs( const device_e pDevID ) const {
    return (
        (pDevID == device_e::keyboard) ? &keyboard.diffs[0] : (
        (pDevID == device_e::mouse) ? &mouse.diffs[0] : nullptr ));
}


bool32_t input_t::isDiffRaw( diff_f pDiff, const uint32_t pInputGID ) const {
    return pDiff( this, pInputGID );
}


uint32_t input_t::isDiffRaw( diff_f pDiff, const uint32_t* pInputGIDs ) const {
    uint32_t firstIdx = INPUT_UNBOUND_ID;
    for( uint32_t inputIdx = 0;
            pInputGIDs != nullptr &&
            pInputGIDs[inputIdx] != INPUT_UNBOUND_ID &&
            firstIdx == INPUT_UNBOUND_ID;
            inputIdx++ ) {
        firstIdx = pDiff( this, pInputGIDs[inputIdx] ) ? pInputGIDs[inputIdx] : INPUT_UNBOUND_ID;
    }
    return firstIdx;
}


uint32_t input_t::isDiffAct( diff_f pDiff, const uint32_t pInputAction ) const {
    return isDiffRaw( pDiff, binding.find(pInputAction) );
}


uint32_t input_t::isDiffAct( diff_f pDiff, const uint32_t* pInputActions ) const {
    uint32_t firstAction = ACTION_UNBOUND_ID;
    for( uint32_t actionIdx = 0;
            pInputActions[actionIdx] != ACTION_UNBOUND_ID &&
            firstAction == ACTION_UNBOUND_ID;
            actionIdx++ ) {
        firstAction = isDiffRaw( pDiff, binding.find(pInputActions[actionIdx]) ) ?
            pInputActions[actionIdx] : ACTION_UNBOUND_ID;
    }
    return firstAction;
}

/// Namespace Functions ///

bool32_t isDown( const input_t* pInput, const uint32_t pInputGID ) {
    const stream_t cInputStream( pInputGID );
    const uint8_t* cInputState = pInput->state( cInputStream.mDevID );
    return (bool32_t)( cInputState != nullptr && cInputState[cInputStream.mID] );
}


bool32_t isPressed( const input_t* pInput, const uint32_t pInputGID ) {
    const stream_t cInputStream( pInputGID );
    const diff_e* cInputDiffs = pInput->diffs( cInputStream.mDevID );
    return (bool32_t)( cInputDiffs != nullptr && (cInputDiffs[cInputStream.mID] == diff_e::down) );
}


bool32_t isReleased( const input_t* pInput, const uint32_t pInputGID ) {
    const stream_t cInputStream( pInputGID );
    const diff_e* cInputDiffs = pInput->diffs( cInputStream.mDevID );
    return (bool32_t)( cInputDiffs != nullptr && (cInputDiffs[cInputStream.mID] == diff_e::up) );
}

}

}
