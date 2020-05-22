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
        itemDelta -= 1;
    } if( llce::input::isKeyPressed(pInput, SDL_SCANCODE_S) ) {
        itemDelta += 1;
    } if( llce::input::isKeyPressed(pInput, SDL_SCANCODE_I) ) {
        itemDelta -= 1;
    } if( llce::input::isKeyPressed(pInput, SDL_SCANCODE_K) ) {
        itemDelta += 1;
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
        const static float32_t csHeaderPadding = 0.05f;
        const static vec2f32_t csHeaderDims = { 1.0f - 2.0f * csHeaderPadding, 0.25f };
        const static vec2f32_t csHeaderPos = { csHeaderPadding, 1.0f - csHeaderPadding - csHeaderDims.y };

        llce::gfx::color_context_t headerCC( mTitleColor );
        llce::gfx::render::text( mTitle, llce::box_t(csHeaderPos, csHeaderDims) );
    }

    { // Items //
        const static llce::box_t csItemArea( 0.0f, 0.0f, 1.0f, 0.5f );
        const static vec2f32_t csItemBase = csItemArea.at( llce::geom::anchor2D::lh );
        const static float32_t csItemPadFactor = 1.50f;
        const static float32_t csItemMaxHeight = 0.10f;

        const float32_t cItemFitWidth = 1.0f;
        const float32_t cItemFitHeight = csItemArea.mDims.y / ( csItemPadFactor * mItemCount );
        const float32_t cItemWidth = cItemFitWidth;
        const float32_t cItemHeight = glm::min( csItemMaxHeight, cItemFitHeight );
        const vec2f32_t cItemDims( cItemWidth, cItemHeight );

        for( uint32_t itemIdx = 0; itemIdx < mItemCount; itemIdx++ ) {
            vec2f32_t itemPos = csItemBase -
                vec2f32_t( 0.0f, itemIdx * csItemPadFactor * cItemHeight );

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
