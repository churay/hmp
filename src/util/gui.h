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
    // FIXME(JRC): Changing this value to 32 causes weird things to happen.
    constexpr static uint32_t MAX_ITEM_COUNT = 16;

    menu_t();
    menu_t(
        llce::input::input_t* pInput, const uint32_t* pEventActions,
        const char8_t* pTitle, const char8_t** pItems, uint32_t pItemCount,
        const color4u8_t* pColorBack, const color4u8_t* pColorFore, const color4u8_t* pColorText );

    void update( const float64_t pDT );
    void render() const;

    bool32_t changed( const llce::gui::event_e pEvent ) const;

    llce::input::input_t* mInput;
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


struct bind_menu_t : public menu_t {
    constexpr static float32_t MAX_ITEMS_VISIBLE = 4.5f;
    constexpr static float32_t ITEM_PART_VISIBILITY = 0.5f;
    constexpr static uint8_t ITEM_FULL_COUNT = 4;

    bind_menu_t();
    bind_menu_t(
        llce::input::input_t* pInput, const uint32_t* pEventActions,
        const char8_t** pActionNames, uint32_t pActionCount,
        const color4u8_t* pColorBack, const color4u8_t* pColorFore,
        const color4u8_t* pColorText, const color4u8_t* pColorBorder );

    void update( const float64_t pDT );
    void render() const;

    llce::deque<uint32_t, LLCE_MAX_BINDINGS> mCurrBindings;
    bool8_t mBinding;
    bool8_t mListening;

    uint8_t mRenderIndex;

    const color4u8_t* mColorBorder;
};

/// Namespace Functions ///

};

};

#endif
