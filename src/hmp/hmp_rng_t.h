#ifndef HMP_RNG_T_H
#define HMP_RNG_T_H

#include <sstream>

#include "consts.h"

namespace hmp {

class rng_t {
    public:

    /// Constructors ///

    rng_t( const uint64_t pSeed );

    /// Class Functions ///

    uint64_t next();

    /// Class Fields ///

    uint64_t mSeed;
};

}

// std::ostream& operator<<( std::ostream& pOS, const hmp::rng_t& pRNG );

#endif
