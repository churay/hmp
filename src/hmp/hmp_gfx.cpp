#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_opengl_glext.h>

#include <glm/geometric.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "hmp_gfx.h"

namespace hmp {

namespace gfx {

/// 'hmp::gfx::render_context_t' Functions ///

render_context_t::render_context_t( const box_t& pBox, const color32_t* pColor ) {
    glPushMatrix();
    glm::mat4 matModelWorld( 1.0f );
    matModelWorld *= glm::translate( glm::mat4(1.0f), glm::vec3(pBox.mPos.x, pBox.mPos.y, 0.0f) );
    matModelWorld *= glm::scale( glm::mat4(1.0f), glm::vec3(pBox.mDims.x, pBox.mDims.y, 0.0f) );
    glMultMatrixf( &matModelWorld[0][0] );

    glPushAttrib( GL_CURRENT_BIT );
    glColor4ubv( (uint8_t*)pColor );
}


render_context_t::~render_context_t() {
    glPopAttrib();
    glPopMatrix();
}


void render_context_t::render() const {
    glBegin( GL_QUADS ); {
        glVertex2f( 0.0f, 0.0f );
        glVertex2f( 1.0f, 0.0f );
        glVertex2f( 1.0f, 1.0f );
        glVertex2f( 0.0f, 1.0f );
    } glEnd();
}

/// 'hmp::gfx::fbo_context_t' Functions ///

fbo_context_t::fbo_context_t( const uint32_t pFBID, const vec2u32_t pFBRes ) {
    int32_t viewParams[4];
    glGetIntegerv( GL_VIEWPORT, &viewParams[0] );
    mViewCoords = { static_cast<uint32_t>(viewParams[0]), static_cast<uint32_t>(viewParams[1]) };
    mViewRes = { static_cast<uint32_t>(viewParams[2]), static_cast<uint32_t>(viewParams[3]) };

    glBindFramebuffer( GL_FRAMEBUFFER, pFBID );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glViewport( 0, 0, pFBRes.x, pFBRes.y );
}


fbo_context_t::~fbo_context_t() {
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    glViewport( mViewCoords.x, mViewCoords.y, mViewRes.x, mViewRes.y );
}

};

};
