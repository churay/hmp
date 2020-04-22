#ifndef LLCE_UI_H
#define LLCE_UI_H

#include <glm/glm.hpp>

#include "input.h"
#include "gfx.h"

#include "consts.h"

namespace llce {

namespace gui {

/// Namespace Attributes ///

/// Namespace Types ///

struct menu_t {
    constexpr static uint32_t MAX_ITEM_LENGTH = 32;
    constexpr static uint32_t MAX_ITEM_COUNT = 16;

    enum class action_e : uint8_t { none = 0, select = 1, up = 2, down = 3 };

    menu_t();
    menu_t( const char8_t* pTitle, const char8_t** pItems, uint32_t pItemCount,
        const color4u8_t* pColor, const color4u8_t* pTitleColor,
        const color4u8_t* pItemColor, const color4u8_t* pSelectColor );

    action_e update( const llce::input::keyboard_t* pInput, const float64_t pDT );
    void render() const;

    char8_t mTitle[MAX_ITEM_LENGTH];
    char8_t mItems[MAX_ITEM_COUNT][MAX_ITEM_LENGTH];
    uint8_t mItemCount;
    uint8_t mSelectIndex;

    const color4u8_t* mColor;
    const color4u8_t* mTitleColor;
    const color4u8_t* mItemColor;
    const color4u8_t* mSelectColor;
};

/// Namespace Functions ///

};

};

#endif
