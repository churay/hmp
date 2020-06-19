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
    llce::input::input_t* pInput, const uint32_t* pEventActions,
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

/// 'llce::gui::bind_menu_t' Functions ///

bind_menu_t::bind_menu_t() : menu_t(),
        mBinding( false ), mListening( false ), mRenderIndex( 0 ), mColorBorder( nullptr ) {
    
}


bind_menu_t::bind_menu_t(
    llce::input::input_t* pInput, const uint32_t* pEventActions,
    const char8_t** pActionNames, uint32_t pActionCount,
    const color4u8_t* pColorBack, const color4u8_t* pColorFore,
    const color4u8_t* pColorText, const color4u8_t* pColorBorder ) :
        menu_t( pInput, pEventActions, "BINDING", pActionNames, pActionCount + 1, pColorBack, pColorFore, pColorText ),
        mBinding( false ), mListening( false ), mRenderIndex( 0 ), mColorBorder( pColorBorder ) {
    std::strncpy( &mItems[pActionCount][0], "EXIT", MAX_ITEM_LENGTH - 1 );
    mItems[pActionCount][MAX_ITEM_LENGTH - 1] = '\0';
}


void bind_menu_t::update( const float64_t pDT ) {
    if( !mBinding ) {
        menu_t::update( pDT );
        mBinding = mItemIndex != mItemCount - 1 && changed( llce::gui::event::select );
        mRenderIndex = (
            (mItemIndex >= mRenderIndex + ITEM_FULL_COUNT) ? mItemIndex - ITEM_FULL_COUNT + 1 : (
            (mItemIndex < mRenderIndex) ? mItemIndex : (mRenderIndex)) );
    } else if( !mListening ) {
        bool8_t inputActive = false;
        for( uint32_t deviceID = llce::input::device::keyboard;
                deviceID <= llce::input::device::_length;
                deviceID++ ) {
            for( uint32_t streamID = 0;
                    streamID < llce::input::SDL_NUM_DEVCODES[deviceID];
                    streamID++ ) {
                llce::input::stream_t stream( deviceID, streamID );
                inputActive |= mInput->isDownRaw( stream );
            }
        }

        if( (mListening = !inputActive) ) {
            mInput->mBinding.unbind( mItemIndex + 1 );
        }
    } else {
        bool8_t inputActive = false;
        for( uint32_t deviceID = llce::input::device::keyboard;
                deviceID <= llce::input::device::_length;
                deviceID++ ) {
            for( uint32_t streamID = 0;
                    streamID < llce::input::SDL_NUM_DEVCODES[deviceID];
                    streamID++ ) {
                llce::input::stream_t stream( deviceID, streamID );
                inputActive |= mInput->isDownRaw( stream );

                if( mInput->isReleasedRaw(stream) ) {
                    mCurrBindings.push_back( stream );
                }
            }
        }

        if( !mCurrBindings.empty() && !inputActive ) {
            uint32_t bindingBuffer[LLCE_MAX_BINDINGS + 1];
            for( uint32_t bindingIdx = 0; bindingIdx < mCurrBindings.size(); bindingIdx++ ) {
                bindingBuffer[bindingIdx] = mCurrBindings.front( bindingIdx );
            }
            bindingBuffer[mCurrBindings.size()] = llce::input::INPUT_UNBOUND_ID;

            mInput->mBinding.bind( mItemIndex + 1, &bindingBuffer[0] );

            mCurrBindings.clear();
            mBinding = false;
            mListening = false;
        }
    }
}


void bind_menu_t::render() const {
    llce::gfx::color_context_t menuCC( mColorBack );
    llce::gfx::render::box();

    const static auto csPadAnchor = llce::geom::anchor2D::mm;

    { // Header //
        const static float32_t csHeaderPadding = 2.0e-2f;
        const static vec2f32_t csHeaderDims = { 1.0f, 0.2f };
        const static vec2f32_t csHeaderInnerDims =
            csHeaderDims - 2.0f * csHeaderPadding * vec2f32_t( 1.0f, 1.0f );
        const static vec2f32_t csHeaderPos = { 0.5f, 1.0f - 0.2f * 0.5f };

        llce::gfx::color_context_t headerCC( mColorText );
        llce::gfx::render::text( mTitle,
            llce::box_t(csHeaderPos, csHeaderInnerDims, csPadAnchor) );
    }

    { // Items //
        const static float32_t csItemPadding = 2.0e-2f;
        const static llce::box_t csItemArea( 0.0f, 0.0f, 1.0f, 0.8f );
        const static vec2f32_t csItemDims =
            { csItemArea.mDims.x, csItemArea.mDims.y / MAX_ITEMS_VISIBLE };
        const static vec2f32_t csItemInnerDims =
            csItemDims - 2.0f * csItemPadding * vec2f32_t( 1.0f, 1.0f );

        const static vec2f32_t csItemSectionDims = { 0.5f - 2.0f * csItemPadding, 1.0f };
        const static vec2f32_t csItemTextDims =
            vec2f32_t( 1.0f, 1.0f ) - 2.0f * csItemPadding * vec2f32_t( 1.0f, 1.0f );
        const static vec2f32_t csItemActionPos = { 0.25f, 0.5f };
        const static vec2f32_t csItemBindingPos = { 0.75f, 0.5f };
        const static vec2f32_t csItemBase = csItemArea.at( llce::geom::anchor2D::lh );

        llce::gfx::color_context_t footerCC( mColorBorder );
        // TODO(JRC): Need to adjust 'mRenderIndex' offset so that interim
        // windows display 25% of off-window before and after items, while
        // boundary windows just display 50% of the internal off-window option.
        // const vec2f32_t cItemRenderOffset = vec2f32_t( 0.0f, csItemDims.y ) * (
        //     (mRenderIndex == 0) ? 0.0f : (
        //     (mRenderIndex == mItemCount - 1) ? ITEM_PART_VISIBILITY : (
        //     ITEM_PART_VISIBILITY / 2.0f )) );
        for( uint32_t itemIdx = 0; itemIdx < mItemCount; itemIdx++ ) {
            vec2f32_t itemPos = csItemBase
                - vec2f32_t( 0.0f, (itemIdx - mRenderIndex) * csItemDims.y )
                + 0.5f * vec2f32_t( csItemDims.x, -csItemDims.y );

            if( itemIdx == mItemIndex ) {
                footerCC.update( mColorBorder );
                llce::gfx::render::box(
                    llce::box_t(itemPos, csItemDims, csPadAnchor) );
            }

            llce::gfx::render_context_t itemRC(
               llce::box_t(itemPos, csItemInnerDims, csPadAnchor) );

            if( itemIdx == mItemCount - 1 ) {
                llce::gfx::render_context_t exitActionRC(
                   llce::box_t(vec2f32_t(0.5f, 0.5f), csItemTextDims, csPadAnchor) );

                footerCC.update( mColorFore );
                llce::gfx::render::box();

                footerCC.update( mColorText );
                llce::gfx::render::text( mItems[itemIdx],
                    llce::box_t(vec2f32_t(0.5f, 0.5f), csItemTextDims, csPadAnchor) );
            } else {
                { // Item Action Section //
                    llce::gfx::render_context_t itemActionRC(
                       llce::box_t(csItemActionPos, csItemSectionDims, csPadAnchor) );

                    footerCC.update( mColorFore );
                    llce::gfx::render::box();

                    footerCC.update( mColorText );
                    llce::gfx::render::text( mItems[itemIdx],
                        llce::box_t(vec2f32_t(0.5f, 0.5f), csItemTextDims, csPadAnchor) );
                }

                { // Item Binding Section //
                    llce::gfx::render_context_t itemBindingRC(
                       llce::box_t(csItemBindingPos, csItemSectionDims, csPadAnchor) );

                    footerCC.update( mColorFore );
                    llce::gfx::render::box();

                    char8_t bindingNames[LLCE_MAX_BINDINGS][16];
                    uint32_t bindingIdx = 0, bindingCount = 0, bindingLength = 0;
                    const uint32_t* cItemBinding = mInput->mBinding.find( itemIdx + 1 );
                    for( bindingIdx = 0;
                            cItemBinding[bindingIdx] != llce::input::INPUT_UNBOUND_ID;
                            bindingIdx++, bindingCount++ ) {
                        llce::input::identify( cItemBinding[bindingIdx],
                            &bindingNames[bindingIdx][0], 16 );
                        bindingLength += std::strlen( &bindingNames[bindingIdx][0] );
                    } for( ; bindingIdx < LLCE_MAX_BINDINGS; bindingIdx++ ) {
                        std::strcpy( &bindingNames[bindingIdx][0], "" );
                    }

                    // TODO(JRC): The following code naively segments the output GUI
                    // space so that bindings are widely spread; this should be improved
                    // so that the bindings are all centered and only small interrim
                    // distances are maintained.
                    float32_t bindingGUIPos = 0.0f;
                    for( bindingIdx = 0; bindingIdx < bindingCount; bindingIdx++ ) {
                        const char8_t* bindingName = &bindingNames[bindingIdx][0];
                        float32_t bindingGUILength = std::strlen( bindingName ) / ( bindingLength + 0.0f );

                        footerCC.update( mColorText );
                        llce::gfx::render_context_t bindingRC(
                           llce::box_t(bindingGUIPos, 0.0f, bindingGUILength, 1.0f) );
                        llce::gfx::render::text( bindingName,
                            llce::box_t(vec2f32_t(0.5f, 0.5f), csItemTextDims, csPadAnchor) );

                        bindingGUIPos += bindingGUILength;
                    }
                }
            }
        }
    }
}

};

};
