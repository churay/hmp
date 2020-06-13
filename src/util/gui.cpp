#include <SDL2/SDL.h>

#include <cstring>

#include "input.h"
#include "gfx.h"

#include "gui.h"

namespace llce {

namespace gui {

/// 'llce::gui::menu_t' Functions ///

menu_t::menu_t() :
        mInput( nullptr ), mTitle( "" ), mItemCount( 0 ), mItemIndex( 0 ),
        mColorBack( nullptr ), mColorFore( nullptr ), mColorText( nullptr ) {
    
}


menu_t::menu_t(
    const llce::input::input_t* pInput, const uint32_t* pEventActions,
    const char8_t* pTitle, const char8_t** pItems, uint32_t pItemCount,
    const color4u8_t* pColorBack, const color4u8_t* pColorFore, const color4u8_t* pColorText ) :
        mInput( pInput ), mItemCount( pItemCount ), mItemIndex( 0 ),
        mColorBack( pColorBack ), mColorFore( pColorFore ), mColorText( pColorText ) {
    LLCE_ASSERT_ERROR( pItemCount <= MAX_ITEM_COUNT,
        "Unable to create menu with " << pItemCount << " items; " << 
        "the maximum number of supported items is " << MAX_ITEM_COUNT << "." );

    for( uint32_t actionIdx = 0; actionIdx < llce::gui::event::_length; actionIdx++ ) {
        mEventActions[actionIdx] = pEventActions[actionIdx];
    }

    std::strncpy( mTitle, pTitle, MAX_ITEM_LENGTH - 1 );
    mTitle[MAX_ITEM_LENGTH - 1] = '\0';

    for( uint32_t itemIdx = 0; itemIdx < mItemCount; itemIdx++ ) {
        std::strncpy( mItems[itemIdx], pItems[itemIdx], MAX_ITEM_LENGTH - 1 );
        mItems[itemIdx][MAX_ITEM_LENGTH - 1] = '\0';
    }
}


void menu_t::update( const float64_t pDT ) {
    int8_t itemDelta = 0;

    {
        mEvents.clear();

        if( mInput->isPressedAct(mEventActions[llce::gui::event::select]) ) {
            mEvents.push_back( llce::gui::event::select );
        } if( mInput->isPressedAct(mEventActions[llce::gui::event::next]) ) {
            itemDelta += 1;
        } if( mInput->isPressedAct(mEventActions[llce::gui::event::prev]) ) {
            itemDelta -= 1;
        }

        if( itemDelta > 0 ) {
            mEvents.push_back( llce::gui::event::next );
        } else if( itemDelta < 0 ) {
            mEvents.push_back( llce::gui::event::prev );
        }
    }

    if( !changed(llce::gui::event::select) ) {
        int8_t newItemIndex = ( mItemIndex + itemDelta ) % mItemCount;
        mItemIndex = ( newItemIndex >= 0 ) ? newItemIndex : mItemCount + newItemIndex;
    }
}


void menu_t::render() const {
    llce::gfx::color_context_t menuCC( mColorBack );
    llce::gfx::render::box();

    { // Header //
        const static float32_t csHeaderPadding = 0.05f;
        const static vec2f32_t csHeaderDims = { 1.0f - 2.0f * csHeaderPadding, 0.25f };
        const static vec2f32_t csHeaderPos = { csHeaderPadding, 1.0f - csHeaderPadding - csHeaderDims.y };

        llce::gfx::color_context_t headerCC( mColorText );
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
            llce::gfx::color_context_t itemCC( mColorFore );
            if( itemIdx == mItemIndex ) { llce::gfx::render::box(); }

            llce::gfx::color_context_t textCC( mColorText );
            llce::gfx::render::text( mItems[itemIdx] );
        }
    }
}


bool32_t menu_t::changed( const llce::gui::event_e pEvent ) const {
    bool32_t found = false;

    for( uint32_t eventIdx = 0; eventIdx < mEvents.size() && !found; eventIdx++ ) {
        found |= mEvents.front( eventIdx ) == pEvent;
    }

    return found;
}

};

};
