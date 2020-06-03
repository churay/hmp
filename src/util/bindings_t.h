#ifndef LLCE_BINDINGS_T_H
#define LLCE_BINDINGS_T_H

#include <SDL2/SDL.h>

#include "input.h"
#include "consts.h"

namespace llce {

namespace input {

struct binding_t { uint32_t* inputs; uint32_t size; };

uint64_t global2device( const uint32_t pGlobalInputID );
uint32_t device2global( const device_e pDeviceID, const uint32_t pDeviceInputID );

class bindings_t {
    public:

    /// Class Attributes ///

    constexpr static uint32_t MAX_ACTION_BINDINGS = 4;
    constexpr static uint8_t UNBOUND_ID = 0;

    /// Constructors ///

    bindings_t();
    bindings_t( const uint32_t* pActionIDs, const uint32_t* pGlobalInputIDs, const uint32_t pInputSize );

    /// Class Functions ///

    bool32_t bind( const uint32_t pActionID, const uint32_t* pGlobalInputIDs, const uint32_t pGlobalInputSize );
    binding_t find( const uint32_t pActionID );

    /// Class Fields ///

    private:

    // sim-specific action id => [bound sim-agnostic input global id]
    uint32_t mActionBindings[LLCE_MAX_ACTIONS][MAX_ACTION_BINDINGS];
    uint32_t mActionBindingSizes[LLCE_MAX_ACTIONS];
    // sim-agnostic global id => sim-specific action id
    uint32_t mBoundActions[SDL_NUM_INPUTS + 1];
};

}

}

#endif
