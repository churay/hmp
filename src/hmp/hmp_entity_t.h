#ifndef HMP_ENTITY_T_H
#define HMP_ENTITY_T_H

#include "hmp_box_t.h"

#include "consts.h"

namespace hmp {

class entity_t {
    public:

    /// Constructors ///

    entity_t( const box_t& pBBox );

    /// Class Functions ///

    void update( const float64_t pDT );
    void render() const;

    /// Internal Functions ///

    private:

    virtual void iupdate( const float64_t pDT ) = 0;
    virtual void irender() const = 0;

    /// Class Fields ///

    public:

    box_t mBBox; // units: world
    float64_t mLifetime; // units: seconds
};

}

#endif
