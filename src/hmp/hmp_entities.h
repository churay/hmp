#ifndef HMP_ENTITIES_T_H
#define HMP_ENTITIES_T_H

#include <glm/ext/scalar_constants.hpp>
#include <glm/ext/vector_float2.hpp>

#include "hmp_entity_t.h"
#include "box_t.h"

#include "hmp_consts.h"
#include "consts.h"

namespace hmp {

class team_entity_t : public entity_t {
    public:

    /// Constructors ///

    team_entity_t( const llce::box_t& pBBox, const team::team_e& pTeam );

    /// Class Functions ///

    void change( const team::team_e& pTeam );

    /// Class Fields ///

    public:

    uint8_t mTeam;
};

class bounds_t : public entity_t {
    public:

    /// Class Attributes ///

    constexpr static float32_t LINE_WIDTH = 5.0e-2f;

    /// Constructors ///

    bounds_t( const llce::box_t& pBBox );

    /// Class Functions ///

    void render() const;

    /// Class Fields ///

    public:
};


class ball_t : public team_entity_t {
    public:

    /// Class Attributes ///

    constexpr static float32_t MOVE_VEL = 5.0e-1f; // units: world / seconds
    constexpr static float32_t HINT_VEL = MOVE_VEL / 5.0e1f; // units: world / seconds
    constexpr static float32_t MAX_RICOCHET_ANGLE = ( 3.0f / 5.0f ) * ( M_PI / 2.0f ); // units: radians

    /// Constructors ///

    ball_t( const llce::box_t& pBBox );

    /// Class Functions ///

    void ricochet( const entity_t* pSurface );

    /// Class Fields ///

    public:
};


class paddle_t : public team_entity_t {
    public:

    /// Class Attributes ///

    constexpr static float32_t MOVE_VEL = 1.0e0f; // units: world / seconds

    /// Constructors ///

    paddle_t( const llce::box_t& pBBox, const team::team_e& pTeam );

    /// Class Functions ///

    void update( const float64_t pDT );
    void move( const int32_t pDX, const int32_t pDY );


    /// Class Fields ///

    public:

    int8_t mDX, mDY;
};


class scoreboard_t : public entity_t {
    public:

    /// Class Attributes ///

    constexpr static float32_t PADDING_WIDTH = 1.0e-2f;
    constexpr static float32_t TALLY_RADIUS = 1.0e-1f;

    /// Constructors ///

    scoreboard_t( const llce::box_t& pBBox );

    /// Class Functions ///

    void render() const;
    void tally( const int8_t pWestDelta, const int8_t pEastDelta );

    /// Class Fields ///

    public:

    int8_t mScores[2];
};

}

#endif
