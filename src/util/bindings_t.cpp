#include <cstring>

#include <SDL2/SDL.h>

#include "bindings_t.h"

namespace llce {

namespace input {

/// Namespace Functions ///

uint64_t global2device( const uint32_t pGlobalInputID ) {
    uint64_t deviceID = 0, deviceOffset = SDL_NUM_DEVCODES[0];
    while( deviceOffset < pGlobalInputID ) {
        deviceOffset += SDL_NUM_DEVCODES[++deviceID];
    }

    uint64_t inputID = pGlobalInputID - ( deviceOffset - SDL_NUM_DEVCODES[deviceID] );
    return ( deviceID << 32 ) | ( inputID << 0 );
}


uint32_t device2global( const device_e pDeviceID, const uint32_t pDeviceInputID ) {
    uint32_t deviceOffset = 0;
    for( uint32_t deviceID = 0; deviceID < (uint32_t)pDeviceID; deviceID++ ) {
        deviceOffset += SDL_NUM_DEVCODES[deviceID];
    }

    return deviceOffset + pDeviceInputID;
}

/// Class Functions ///

bindings_t::bindings_t() {
    std::memset( &mActionBindings[0], bindings_t::UNBOUND_ID, sizeof(mActionBindings) );
    std::memset( &mActionBindingSizes[0], 0, sizeof(mActionBindingSizes) );
    std::memset( &mBoundActions[0], bindings_t::UNBOUND_ID, sizeof(mBoundActions) );
}


// TODO(JRC): This signature limits default configurations to one binding per
// action. This should be extended to arbitrary bindings per action (provided
// such a change doesn't overcomplicate the prototype).
bindings_t::bindings_t( const uint32_t* pActionIDs, const uint32_t* pGlobalInputIDs, const uint32_t pInputSize ) :
        bindings_t() {
    for( uint32_t inputIdx = 0; inputIdx < pInputSize; inputIdx++ ) {
        mActionBindings[pActionIDs[inputIdx]][0] = pGlobalInputIDs[inputIdx];
        mActionBindingSizes[pActionIDs[inputIdx]] = 1;
    }
}


bool32_t bindings_t::bind( const uint32_t pActionID, const uint32_t* pGlobalInputIDs, const uint32_t pGlobalInputSize ) {
    bool32_t validBind = true;

    LLCE_VERIFY_ERROR( validBind &= (pActionID < LLCE_MAX_ACTIONS),
        "Invalid action ID '" << pActionID << "'; "
        "valid range is [0, " << LLCE_MAX_ACTIONS << ")." );
    LLCE_VERIFY_ERROR( validBind &= (pGlobalInputSize > 0),
        "Binding no inputs to an action; each bound action must have at least " <<
        "one corresponding input." );
    LLCE_VERIFY_ERROR( validBind &= (pGlobalInputSize <= MAX_ACTION_BINDINGS),
        "Binding too many inputs " << pGlobalInputSize << " to a single action; " <<
        "the maximum number of bindings is " << MAX_ACTION_BINDINGS << "." );

    if( validBind ) {
        const static uint32_t csActionBytes = MAX_ACTION_BINDINGS * sizeof( uint32_t );
        std::memset( &mActionBindings[pActionID], bindings_t::UNBOUND_ID, csActionBytes );

        const uint32_t cInputBytes = pGlobalInputSize * sizeof( uint32_t );
        std::memcpy( &mActionBindings[pActionID], pGlobalInputIDs, cInputBytes );
        mActionBindingSizes[pActionID] = pGlobalInputSize;
    }

    return validBind;
}


binding_t bindings_t::find( const uint32_t pActionID ) {
    bool32_t isBound = mActionBindingSizes[pActionID] > 0;
    return {
        isBound ? &mActionBindings[pActionID][0] : nullptr,
        isBound ? mActionBindingSizes[pActionID] : 0
    };
}

}

}
