#include <cmath>
#include <cstring>

#include "input.h"

namespace llce {

namespace input {

/// 'llce::input::stream_t' Functions ///

stream_t::stream_t( device_e pDevID, uint32_t pID ) :
        mDevID( pDevID ), mID( pID ) {
    
}


stream_t::stream_t( uint32_t pDevID, uint32_t pID ) :
        mDevID( static_cast<device_e>(pDevID) ), mID( pID ) {
    
}


stream_t::stream_t( uint64_t pGlobalID ) :
        mDevID( static_cast<device_e>(pGlobalID >> 32) ), mID( pGlobalID >> 0 ) {
    
}


uint32_t stream_t::gid() const {
    return static_cast<uint32_t>(*this);
}


uint64_t stream_t::did() const {
    return static_cast<uint64_t>(*this);
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
                bindingIdx < LLCE_MAX_BINDINGS && pInputGIDs[bindingIdx];
                bindingIdx++ ) {
            mActionBindings[pActionID][bindingIdx] = pInputGIDs[bindingIdx];
        }
    }

    return validBind;
}


uint32_t* binding_t::find( const uint32_t pActionID ) {
    return &mActionBindings[pActionID][0];
}

/// Namespace Functions ///

bool32_t readKeyboard( keyboard_t* pKeyboard ) {
    bool32_t readSuccessful = false;

    if( pKeyboard != nullptr ) {
        const uint8_t* keyboardState = SDL_GetKeyboardState( nullptr );

        for( uint32_t keyIdx = 0; keyIdx < sizeof(pKeyboard->keys); keyIdx++ ) {
            const bool8_t wasKeyDown = pKeyboard->keys[keyIdx];
            const bool8_t isKeyDown = keyboardState[keyIdx];

            pKeyboard->keys[keyIdx] = isKeyDown;
            pKeyboard->diffs[keyIdx] = (
                (!wasKeyDown && isKeyDown) ? diff_e::down : (
                (wasKeyDown && !isKeyDown) ? diff_e::up : (
                diff_e::none)) );
        }

        readSuccessful = true;
    }

    return readSuccessful;
}


bool32_t readMouse( mouse_t* pMouse ) {
    bool32_t readSuccessful = false;

    if( pMouse != nullptr ) {
        // NOTE(JRC): The global mouse state will report the state of the mouse
        // regardless of where it's located on the screen where the window mouse
        // state will only report buttons pressed while the mouse is in focus.
        // This being the case, the window button mask is preferred as user input.
        const uint32_t cWindowButtonMask = SDL_GetMouseState( &pMouse->window.x, &pMouse->window.y );
        SDL_GetGlobalMouseState( &pMouse->global.x, &pMouse->global.y );

        for( uint32_t buttonIdx = 1; buttonIdx < sizeof(pMouse->buttons); buttonIdx++ ) {
            const bool8_t wasButtonDown = pMouse->buttons[buttonIdx];
            const bool8_t isButtonDown = cWindowButtonMask & SDL_BUTTON( buttonIdx );

            pMouse->buttons[buttonIdx] = isButtonDown;
            pMouse->diffs[buttonIdx] = (
                (!wasButtonDown && isButtonDown) ? diff_e::down : (
                (wasButtonDown && !isButtonDown) ? diff_e::up : (
                diff_e::none)) );
        }

        // TODO(JRC): Consider readding this field to the 'mouse_t' type should multi-
        // window simulations ever become supported.
        // pMouse->focus = SDL_GetMouseFocus();
        readSuccessful = true;
    }

    return readSuccessful;
}


bool32_t isKeyDown( const keyboard_t* pKeyboard, const SDL_Scancode pKey ) {
    return (bool32_t)( pKeyboard != nullptr && pKeyboard->keys[pKey] );
}


uint32_t isKGDown( const keyboard_t* pKeyboard, const SDL_Scancode* pKeyGroup, const uint32_t pGroupSize ) {
    uint32_t firstIdx = 0;
    for( uint32_t groupIdx = 0; groupIdx < pGroupSize && firstIdx == 0; groupIdx++ ) {
        firstIdx = isKeyDown( pKeyboard, pKeyGroup[groupIdx] ) ? groupIdx + 1 : 0;
    }
    return firstIdx;
}


bool32_t isKeyPressed( const keyboard_t* pKeyboard, const SDL_Scancode pKey ) {
    return (bool32_t)( pKeyboard != nullptr && pKeyboard->keys[pKey] && pKeyboard->diffs[pKey] == diff_e::down );
}


uint32_t isKGPressed( const keyboard_t* pKeyboard, const SDL_Scancode* pKeyGroup, const uint32_t pGroupSize ) {
    uint32_t firstIdx = 0;
    for( uint32_t groupIdx = 0; groupIdx < pGroupSize && firstIdx == 0; groupIdx++ ) {
        firstIdx = isKeyPressed( pKeyboard, pKeyGroup[groupIdx] ) ? groupIdx + 1 : 0;
    }
    return firstIdx;
}


bool32_t isKeyReleased( const keyboard_t* pKeyboard, const SDL_Scancode pKey ) {
    return (bool32_t)( pKeyboard != nullptr && pKeyboard->keys[pKey] && pKeyboard->diffs[pKey] == diff_e::up );
}


uint32_t isKGReleased( const keyboard_t* pKeyboard, const SDL_Scancode* pKeyGroup, const uint32_t pGroupSize ) {
    uint32_t firstIdx = 0;
    for( uint32_t groupIdx = 0; groupIdx < pGroupSize && firstIdx == 0; groupIdx++ ) {
        firstIdx = isKeyReleased( pKeyboard, pKeyGroup[groupIdx] ) ? groupIdx + 1 : 0;
    }
    return firstIdx;
}

}

}
