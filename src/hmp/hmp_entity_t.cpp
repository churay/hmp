#include <cstring>

#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <SDL2/SDL_opengl.h>

#include "gfx.h"
#include "hmp_entity_t.h"

namespace hmp {

/// Class Functions ///

entity_t::entity_t( const llce::box_t& pBBox, const color4u8_t* pColor ) :
        mBBox( pBBox ), mVel( 0.0f, 0.0f ), mColor( pColor ), mLifetime( 0.0f ) {
    
}


void entity_t::update( const float64_t pDT ) {
    mLifetime += pDT;
    mBBox.mPos += static_cast<float32_t>( pDT ) * mVel;
}


void entity_t::render() const {
    llce::gfx::color_context_t entityCC( mColor );
    llce::gfx::render::box( mBBox );
}

}
