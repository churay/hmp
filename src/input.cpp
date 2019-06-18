#include <cmath>
#include <cstring>

#include "input.h"

namespace llce {

bool32_t input::readKeyboard( keyboard_t& pKeyboard ) {
    const uint8_t* keyboardState = SDL_GetKeyboardState( nullptr );

    for( uint32_t keyIdx = 0; keyIdx < sizeof(pKeyboard.keys); keyIdx++ ) {
        const bool8_t wasKeyDown = pKeyboard.keys[keyIdx];
        const bool8_t isKeyDown = keyboardState[keyIdx];

        pKeyboard.keys[keyIdx] = isKeyDown;
        pKeyboard.diffs[keyIdx] = (
            (!wasKeyDown && isKeyDown) ? keydiff_e::down : (
            (wasKeyDown && !isKeyDown) ? keydiff_e::up : (
            keydiff_e::none)) );
    }

    return true;
}


bool32_t input::isKeyDown( const input::keyboard_t& pKeyboard, const SDL_Scancode pKey ) {
    return (bool32_t)( pKeyboard.keys[pKey] );
}


uint32_t input::isKGDown( const input::keyboard_t& pKeyboard, const SDL_Scancode* pKeyGroup, const uint32_t pGroupSize ) {
    uint32_t firstIdx = 0;
    for( uint32_t groupIdx = 0; groupIdx < pGroupSize && firstIdx == 0; groupIdx++ ) {
        firstIdx = isKeyDown( pKeyboard, pKeyGroup[groupIdx] ) ? groupIdx + 1 : 0;
    }
    return firstIdx;
}


bool32_t input::isKeyPressed( const input::keyboard_t& pKeyboard, const SDL_Scancode pKey ) {
    return (bool32_t)( pKeyboard.keys[pKey] && pKeyboard.diffs[pKey] == keydiff_e::down );
}


uint32_t input::isKGPressed( const input::keyboard_t& pKeyboard, const SDL_Scancode* pKeyGroup, const uint32_t pGroupSize ) {
    uint32_t firstIdx = 0;
    for( uint32_t groupIdx = 0; groupIdx < pGroupSize && firstIdx == 0; groupIdx++ ) {
        firstIdx = isKeyPressed( pKeyboard, pKeyGroup[groupIdx] ) ? groupIdx + 1 : 0;
    }
    return firstIdx;
}


bool32_t input::isKeyReleased( const input::keyboard_t& pKeyboard, const SDL_Scancode pKey ) {
    return (bool32_t)( pKeyboard.keys[pKey] && pKeyboard.diffs[pKey] == keydiff_e::up );
}


uint32_t input::isKGReleased( const input::keyboard_t& pKeyboard, const SDL_Scancode* pKeyGroup, const uint32_t pGroupSize ) {
    uint32_t firstIdx = 0;
    for( uint32_t groupIdx = 0; groupIdx < pGroupSize && firstIdx == 0; groupIdx++ ) {
        firstIdx = isKeyReleased( pKeyboard, pKeyGroup[groupIdx] ) ? groupIdx + 1 : 0;
    }
    return firstIdx;
}

}
