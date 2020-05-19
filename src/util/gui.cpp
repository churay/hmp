#include <SDL2/SDL.h>

#include <cstring>

#include "input.h"
#include "gfx.h"

#include "gui.h"

namespace llce {

namespace gui {

/// 'llce::ui::menu_t' Functions ///

menu_t::menu_t() :
        mTitle( "" ), mItemCount( 0 ), mSelectIndex( 0 ),
        mColor( nullptr ), mTitleColor( nullptr ), mItemColor( nullptr ), mSelectColor( nullptr ) {
    
}


menu_t::menu_t( const char8_t* pTitle, const char8_t** pItems, uint32_t pItemCount,
    const color4u8_t* pColor, const color4u8_t* pTitleColor,
    const color4u8_t* pItemColor, const color4u8_t* pSelectColor ) :
        mItemCount( pItemCount ), mSelectIndex( 0 ),
        mColor( pColor ), mTitleColor( pTitleColor ),
        mItemColor( pItemColor ), mSelectColor( pSelectColor ) {
    LLCE_ASSERT_ERROR( pItemCount <= MAX_ITEM_COUNT,
        "Unable to create menu with " << pItemCount << " items; " << 
        "the maximum number of supported items is " << MAX_ITEM_COUNT << "." );

    std::strncpy( mTitle, pTitle, MAX_ITEM_LENGTH - 1 );
    mTitle[MAX_ITEM_LENGTH - 1] = '\0';

    for( uint32_t itemIdx = 0; itemIdx < mItemCount; itemIdx++ ) {
        std::strncpy( mItems[itemIdx], pItems[itemIdx], MAX_ITEM_LENGTH - 1 );
        mItems[itemIdx][MAX_ITEM_LENGTH - 1] = '\0';
    }
}


event_e menu_t::update( const llce::input::keyboard_t* pInput, const float64_t pDT ) {
    int8_t itemDelta = 0;
    bool8_t itemSelected = false;

    if( llce::input::isKeyPressed(pInput, SDL_SCANCODE_D) ) {
        itemSelected = true;
    } if( llce::input::isKeyPressed(pInput, SDL_SCANCODE_L) ) {
        itemSelected = true;
    }

    if( llce::input::isKeyPressed(pInput, SDL_SCANCODE_W) ) {
        itemDelta += 1;
    } if( llce::input::isKeyPressed(pInput, SDL_SCANCODE_S) ) {
        itemDelta -= 1;
    } if( llce::input::isKeyPressed(pInput, SDL_SCANCODE_I) ) {
        itemDelta += 1;
    } if( llce::input::isKeyPressed(pInput, SDL_SCANCODE_K) ) {
        itemDelta -= 1;
    }

    event_e status = event_e::none;
    if( itemSelected ) {
        status = event_e::select;
    } else if( itemDelta != 0 ) {
        int8_t newSelectIndex = ( mSelectIndex + itemDelta ) % mItemCount;
        mSelectIndex = ( newSelectIndex >= 0 ) ? newSelectIndex : mItemCount + newSelectIndex;
        status = ( itemDelta < 0 ) ? event_e::prev : event_e::next;
    }

    return status;
}


void menu_t::render() const {
    llce::gfx::color_context_t menuCC( mColor );
    llce::gfx::render::box();

    { // Header //
        const float32_t cHeaderPadding = 0.05f;
        const vec2f32_t cHeaderDims = { 1.0f - 2.0f * cHeaderPadding, 0.25f };
        const vec2f32_t cHeaderPos = { cHeaderPadding, 1.0f - cHeaderPadding - cHeaderDims.y };

        llce::gfx::color_context_t headerCC( mTitleColor );
        llce::gfx::render::text( mTitle, llce::box_t(cHeaderPos, cHeaderDims) );
    }

    { // Items //
        const float32_t cItemPadding = 0.05f;
        const vec2f32_t cItemDims = { 1.0f, 0.10f };
        const vec2f32_t cItemBase = { 0.0f, 0.50f };

        for( uint32_t itemIdx = 0; itemIdx < mItemCount; itemIdx++ ) {
            vec2f32_t itemPos = cItemBase -
                static_cast<float32_t>( itemIdx ) * vec2f32_t( 0.0f, cItemDims.y + cItemPadding );

            llce::gfx::render_context_t itemRC(
                llce::box_t(itemPos, cItemDims, llce::geom::anchor2D::lh) );
            llce::gfx::color_context_t itemCC( mSelectColor );
            if( itemIdx == mSelectIndex ) { llce::gfx::render::box(); }

            llce::gfx::color_context_t textCC( mItemColor );
            llce::gfx::render::text( mItems[itemIdx] );
        }
    }
}

};

};
