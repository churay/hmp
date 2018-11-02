#include <sstream>

#include <SDL2/SDL_opengl.h>

#include "hmp_box_t.h"

namespace hmp {

/// Class Functions ///

box_t::box_t(
    const float32_t pX, const float32_t pY,
    const float32_t pW, const float32_t pH ) :
        mX( pX, pX + pW ), mY( pY, pY + pH ) {
}


void box_t::update( const float32_t pX, const float32_t pY ) {
    mX.mMin += pX;
    mX.mMax += pX;
    mY.mMin += pY;
    mY.mMax += pY;
}


void box_t::render( const uint8_t* pColor ) const {
    glBegin( GL_QUADS ); {
        if( pColor != nullptr ) {
            glColor4ub( pColor[0], pColor[1], pColor[2], pColor[3] );
        } else {
            glColor4ub( 0xFF, 0xFF, 0xFF, 0xFF );
        }

        glVertex2f( mX.mMin, mY.mMin );
        glVertex2f( mX.mMax, mY.mMin );
        glVertex2f( mX.mMax, mY.mMax );
        glVertex2f( mX.mMin, mY.mMax );
    } glEnd();
};


bool32_t box_t::contains( const float32_t pX, const float32_t pY ) const {
    return mX.contains( pX ) && mY.contains( pY );
}


bool32_t box_t::contains( const box_t& pOther ) const {
    return mX.contains( pOther.mX ) && mY.contains( pOther.mY );
}


bool32_t box_t::overlaps( const box_t& pOther ) const {
    return mX.overlaps( pOther.mX ) && mY.overlaps( pOther.mY );
}


bool32_t box_t::empty() const {
    return mX.empty() || mY.empty();
}


bool32_t box_t::valid() const {
    return mX.valid() && mY.valid();
}

}

// std::ostream& operator<<( std::ostream& pOS, const hmp::box_t& pBox ) {
//     pOS << "{X: " << pBox.mX << ", Y: " << pBox.mY << "}";
//     return pOS;
// }
