
#include "geom.h"

namespace llce {

namespace geom {

/// 'llce::geom' Anchor Functions ///

float32_t anchor( const float32_t pInterval, const anchor1D_e pAnchor ) {
    return pInterval * ( static_cast<int32_t>(pAnchor) + 0.0f ) / 2.0f;
}


vec2f32_t anchor( const vec2f32_t pInterval, const anchor2D_e pAnchor ) {
    return vec2f32_t(
        pInterval.x * ( static_cast<int32_t>(pAnchor >> 0 & 0b11) + 0.0f ) / 2.0f,
        pInterval.y * ( static_cast<int32_t>(pAnchor >> 2 & 0b11) + 0.0f ) / 2.0f
    );
}

/// 'llce::geom' Geometry Functions ///

// NOTE(JRC): The source for this algorithm was derived from here:
// http://geomalgorithms.com/a01-_area.html
float32_t determinant( const vec2f32_t& pP0, const vec2f32_t& pP1, const vec2f32_t& pP2 ) {
    return ( (pP1.x - pP0.x) * (pP2.y - pP0.y) - (pP2.x - pP0.x) * (pP1.y - pP0.y) );
}


// NOTE(JRC): The source for this algorithm was derived from here:
// http://geomalgorithms.com/a03-_inclusion.html
bool32_t contains( const vec2f32_t* pPolygon, const uint32_t pPolygonLength, const vec2f32_t& pPoint ) {
    int32_t windCount = 0;

    for( uint32_t cornerIdx = 0; cornerIdx < pPolygonLength; cornerIdx++ ) {
        const vec2f32_t& sideStart = pPolygon[cornerIdx];
        const vec2f32_t& sideEnd = pPolygon[(cornerIdx + 1) % pPolygonLength];

        // if the side edge intersects the point x-axis ray upward...
        if( sideStart.y < pPoint.y && sideEnd.y > pPoint.y ) {
            // and the point is strictly left of the edge (s->e->p is ccw)
            if( llce::geom::determinant(sideStart, sideEnd, pPoint) > 0.0f ) {
                windCount++;
            }
        // if the side edge intersects the point x-axis ray downward...
        } else if( sideStart.y > pPoint.y && sideEnd.y < pPoint.y ) {
            // and the point is strictly right of the edge (s->e->p is cw)
            if( llce::geom::determinant(sideStart, sideEnd, pPoint) < 0.0f ) {
                windCount--;
            }
        }
    }

    return windCount != 0;
}

};

};
