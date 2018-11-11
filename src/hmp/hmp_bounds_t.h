#ifndef HMP_BOUNDS_T_H
#define HMP_BOUNDS_T_H

#include <glm/ext/vector_float2.hpp>

#include "hmp_entity_t.h"
#include "hmp_box_t.h"

#include "consts.h"

namespace hmp {

class bounds_t : public entity_t {
    public:

    /// Constructors ///

    bounds_t( const box_t& pBBox );

    /// Internal Functions ///

    protected:

    virtual void iupdate( const float64_t pDT );
    virtual void irender() const;

    /// Class Fields ///
};

}

#endif
