#include <cmath>
#include <cstring>

#include "input.h"

namespace llce {

bool32_t input::readKeyboard( keyboard_t* pKeyboard ) {
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
    }

    return true;
}


bool32_t input::readMouse( mouse_t* pMouse ) {
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
    }

    return true;
}


bool32_t input::isKeyDown( const input::keyboard_t* pKeyboard, const SDL_Scancode pKey ) {
    return (bool32_t)( pKeyboard != nullptr && pKeyboard->keys[pKey] );
}


uint32_t input::isKGDown( const input::keyboard_t* pKeyboard, const SDL_Scancode* pKeyGroup, const uint32_t pGroupSize ) {
    uint32_t firstIdx = 0;
    for( uint32_t groupIdx = 0; groupIdx < pGroupSize && firstIdx == 0; groupIdx++ ) {
        firstIdx = isKeyDown( pKeyboard, pKeyGroup[groupIdx] ) ? groupIdx + 1 : 0;
    }
    return firstIdx;
}


bool32_t input::isKeyPressed( const input::keyboard_t* pKeyboard, const SDL_Scancode pKey ) {
    return (bool32_t)( pKeyboard != nullptr && pKeyboard->keys[pKey] && pKeyboard->diffs[pKey] == diff_e::down );
}


uint32_t input::isKGPressed( const input::keyboard_t* pKeyboard, const SDL_Scancode* pKeyGroup, const uint32_t pGroupSize ) {
    uint32_t firstIdx = 0;
    for( uint32_t groupIdx = 0; groupIdx < pGroupSize && firstIdx == 0; groupIdx++ ) {
        firstIdx = isKeyPressed( pKeyboard, pKeyGroup[groupIdx] ) ? groupIdx + 1 : 0;
    }
    return firstIdx;
}


bool32_t input::isKeyReleased( const input::keyboard_t* pKeyboard, const SDL_Scancode pKey ) {
    return (bool32_t)( pKeyboard != nullptr && pKeyboard->keys[pKey] && pKeyboard->diffs[pKey] == diff_e::up );
}


uint32_t input::isKGReleased( const input::keyboard_t* pKeyboard, const SDL_Scancode* pKeyGroup, const uint32_t pGroupSize ) {
    uint32_t firstIdx = 0;
    for( uint32_t groupIdx = 0; groupIdx < pGroupSize && firstIdx == 0; groupIdx++ ) {
        firstIdx = isKeyReleased( pKeyboard, pKeyGroup[groupIdx] ) ? groupIdx + 1 : 0;
    }
    return firstIdx;
}

}
