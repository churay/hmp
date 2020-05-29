#ifndef LLCE_UI_H
#define LLCE_UI_H

#include <glm/glm.hpp>

#include "gfx.h"
#include "deque.hpp"

#include "consts.h"

namespace llce {

namespace gui {

/// Namespace Attributes ///

enum class event_e : uint8_t { none = 0, select = 1, next = 2, prev = 3 };

/// Namespace Types ///

struct menu_t {
    constexpr static uint32_t MAX_EVENT_COUNT = 32;
    constexpr static uint32_t MAX_ITEM_LENGTH = 32;
    constexpr static uint32_t MAX_ITEM_COUNT = 16;

    menu_t();
    menu_t( const char8_t* pTitle, const char8_t** pItems, uint32_t pItemCount,
        const color4u8_t* pColor, const color4u8_t* pTitleColor,
        const color4u8_t* pItemColor, const color4u8_t* pSelectColor );

    void update( const float64_t pDT );
    void render() const;

    void submit( const event_e pEvent );

    llce::deque<event_e, MAX_EVENT_COUNT> mEvents;
    char8_t mTitle[MAX_ITEM_LENGTH];
    char8_t mItems[MAX_ITEM_COUNT][MAX_ITEM_LENGTH];
    uint8_t mItemCount;
    uint8_t mSelectIndex;
    bool8_t mSelected;

    const color4u8_t* mColor;
    const color4u8_t* mTitleColor;
    const color4u8_t* mItemColor;
    const color4u8_t* mSelectColor;
};

/// Namespace Functions ///

};

};

#endif
