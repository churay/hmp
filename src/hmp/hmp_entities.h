#ifndef HMP_PADDLE_T_H
#define HMP_PADDLE_T_H

#include <glm/ext/scalar_constants.hpp>
#include <glm/ext/vector_float2.hpp>

#include "hmp_entity_t.h"
#include "hmp_box_t.h"

#include "hmp_consts.h"
#include "consts.h"

namespace hmp {

class bounds_t : public entity_t {
    public:

    /// Class Attributes ///

    constexpr static float32_t LINE_WIDTH = 5.0e-2f;

    /// Constructors ///

    bounds_t( const box_t& pBBox );

    /// Internal Functions ///

    protected:

    virtual void irender() const;

    /// Class Fields ///

    public:
};


class ball_t : public entity_t {
    public:

    /// Class Attributes ///

    constexpr static float32_t MOVE_VEL = 5.0e-1f; // units: world / seconds
    constexpr static float32_t MAX_RICOCHET_ANGLE = ( 3.0f / 5.0f ) * ( M_PI / 2.0f ); // units: radians

    /// Constructors ///

    ball_t( const box_t& pBBox );

    /// Class Functions ///

    void ricochet( const entity_t* pSurface );

    /// Internal Functions ///

    protected:

    /// Class Fields ///

    public:

    team_e mTeam;
};


class paddle_t : public entity_t {
    public:

    /// Class Attributes ///

    constexpr static float32_t MOVE_VEL = 1.0e0f; // units: world / seconds

    /// Constructors ///

    paddle_t( const box_t& pBBox, const team_e& pTeam );

    /// Class Functions ///

    void move( const int32_t pDX, const int32_t pDY );

    /// Internal Functions ///

    protected:

    virtual void iupdate( const float64_t pDT );

    /// Class Fields ///

    public:

    team_e mTeam;
    int8_t mDX, mDY;
};


class scoreboard_t : public entity_t {
    public:

    /// Class Attributes ///

    constexpr static float32_t INNER_PADDING = 1.0e-1f;
    constexpr static float32_t DIGIT_WIDTH = 1.0e-1f;
    constexpr static float32_t LINE_WIDTH = 3.0e-2f;

    /// Constructors ///

    scoreboard_t( const box_t& pBBox );

    /// Class Functions ///

    void tally( const uint8_t pWestDelta, const uint8_t pEastDelta );

    /// Internal Functions ///

    protected:

    virtual void irender() const;

    /// Class Fields ///

    public:

    uint8_t mWestScore, mEastScore;
};

}

#endif
