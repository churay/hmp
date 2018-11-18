#ifndef HMP_PADDLE_T_H
#define HMP_PADDLE_T_H

#include <glm/ext/vector_float2.hpp>

#include "hmp_entity_t.h"
#include "hmp_box_t.h"

#include "hmp_consts.h"
#include "consts.h"

namespace hmp {

class bounds_t : public entity_t {
    public:

    /// Constructors ///

    bounds_t( const box_t& pBBox );

    /// Internal Functions ///

    protected:

    virtual void iupdate( const float64_t pDT );

    /// Class Fields ///
};


class ball_t : public entity_t {
    public:

    /// Class Attributes ///

    constexpr static float32_t MOVE_VEL = 5.0e-1f; // units: world / seconds

    /// Constructors ///

    ball_t( const box_t& pBBox );

    /// Internal Functions ///

    protected:

    virtual void iupdate( const float64_t pDT );

    /// Class Fields ///

    public:

    glm::vec2 mVel; // units: world / seconds
};


class paddle_t : public entity_t {
    public:

    /// Class Attributes ///

    constexpr static float32_t MOVE_VEL = 1.0e0f; // units: world / seconds

    /// Constructors ///

    paddle_t( const box_t& pBBox, const color_t& pColor );

    /// Class Functions ///

    void move( const int32_t pDX, const int32_t pDY );

    /// Internal Functions ///

    protected:

    virtual void iupdate( const float64_t pDT );

    /// Class Fields ///

    public:

    glm::vec2 mVel; // units: world / seconds
    int8_t mDX, mDY;
};

}

#endif
