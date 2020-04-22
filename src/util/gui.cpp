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


menu_t::action_e menu_t::update( const llce::input::keyboard_t* pInput, const float64_t pDT ) {
    int32_t di[2] = { 0, 0 };
    bool32_t dselect = false;

    if( llce::input::isKeyPressed(pInput, SDL_SCANCODE_D) ) {
        dselect = true;
    } if( llce::input::isKeyPressed(pInput, SDL_SCANCODE_L) ) {
        dselect = true;
    }

    if( llce::input::isKeyPressed(pInput, SDL_SCANCODE_W) ) {
        di[0] += 1;
    } if( llce::input::isKeyPressed(pInput, SDL_SCANCODE_S) ) {
        di[0] -= 1;
    } if( llce::input::isKeyPressed(pInput, SDL_SCANCODE_I) ) {
        di[1] += 1;
    } if( llce::input::isKeyPressed(pInput, SDL_SCANCODE_K) ) {
        di[1] -= 1;
    }

    menu_t::action_e status = menu_t::action_e::none;
    if( dselect ) {
        status = menu_t::action_e::select;
    } else if( di[0] + di[1] != 0 ) {
        // TODO(JRC): Improve this code so that it isn't so ugly.
        int8_t newSelectIndex = mSelectIndex + di[0] + di[1];
        mSelectIndex = ( newSelectIndex < 0 ? mItemCount - 1 : newSelectIndex ) % mItemCount;
        status = ( di[0] + di[1] < 0 ) ? menu_t::action_e::down : menu_t::action_e::up;
    }

    return status;
}


void menu_t::render() const {
    llce::gfx::render_context_t menuRC( llce::box_t(0.0f, 0.0f, 1.0f, 1.0f), mColor );
    menuRC.render();

    { // Header //
        const float32_t cHeaderPadding = 0.05f;
        const vec2f32_t cHeaderDims = { 1.0f - 2.0f * cHeaderPadding, 0.25f };
        const vec2f32_t cHeaderPos = { cHeaderPadding, 1.0f - cHeaderPadding - cHeaderDims.y };
        llce::gfx::text::render( mTitle, mTitleColor, llce::box_t(cHeaderPos, cHeaderDims) );
    }

    { // Items //
        const float32_t cItemPadding = 0.05f;
        const vec2f32_t cItemDims = { 1.0f, 0.10f };
        const vec2f32_t cItemBase = { 0.0f, 0.50f };

        for( uint32_t itemIdx = 0; itemIdx < mItemCount; itemIdx++ ) {
            vec2f32_t itemPos = cItemBase -
                static_cast<float32_t>(itemIdx) * vec2f32_t( 0.0f, cItemDims.y + cItemPadding );
            llce::gfx::render_context_t itemRC(
                llce::box_t(itemPos, cItemDims, llce::geom::anchor2D::lh), mSelectColor );

            if( itemIdx == mSelectIndex ) { itemRC.render(); }
            llce::gfx::text::render( mItems[itemIdx], mItemColor );
        }
    }
}

};

};
