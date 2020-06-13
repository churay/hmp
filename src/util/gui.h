#ifndef LLCE_UI_H
#define LLCE_UI_H

#include <glm/glm.hpp>

#include "gfx.h"
#include "input.h"
#include "deque.hpp"

#include "consts.h"

namespace llce {

namespace gui {

/// Namespace Attributes ///

LLCE_ENUM( event, none, select, next, prev );

/// Namespace Types ///

struct menu_t {
    constexpr static uint32_t MAX_EVENT_COUNT = 32;
    constexpr static uint32_t MAX_ITEM_LENGTH = 32;
    constexpr static uint32_t MAX_ITEM_COUNT = 16;

    menu_t();
    menu_t(
        const llce::input::input_t* pInput, const uint32_t* pEventActions,
        const char8_t* pTitle, const char8_t** pItems, uint32_t pItemCount,
        const color4u8_t* pColorBack, const color4u8_t* pColorFore, const color4u8_t* pColorText );

    void update( const float64_t pDT );
    void render() const;

    bool32_t changed( const llce::gui::event_e pEvent ) const;

    const llce::input::input_t* mInput;
    llce::deque<llce::gui::event_e, MAX_EVENT_COUNT> mEvents;
    // TODO(JRC): Allow for the binding of multiple actions to an event (similar
    // to multiple bindings per action).
    uint32_t mEventActions[llce::gui::event::_length];

    char8_t mTitle[MAX_ITEM_LENGTH];
    char8_t mItems[MAX_ITEM_COUNT][MAX_ITEM_LENGTH];
    uint8_t mItemCount;
    uint8_t mItemIndex;

    const color4u8_t* mColorBack;
    const color4u8_t* mColorFore;
    const color4u8_t* mColorText;
};

/// Namespace Functions ///

};

};

#endif
