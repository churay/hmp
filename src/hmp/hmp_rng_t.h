#ifndef HMP_RNG_T_H
#define HMP_RNG_T_H

#include <sstream>

#include "consts.h"

// NOTE(JRC): This random number generator is implemented using the linear
// congruential generator (LCG) architype. In specific, this LCG implements
// MMIX by Donald Knuth, which is a mixed method generator.

namespace hmp {

class rng_t {
    public:

    /// Class Attributes ///

    constexpr static uint64_t MULTIPLIER = 6364136223846793005;
    constexpr static uint64_t INCREMENT = 1442695040888963407;
    constexpr static uint64_t PERIOD = 0xFFFFFFFFFFFFFFFF;

    /// Constructors ///

    rng_t( const uint64_t pSeed );

    /// Class Functions ///

    uint64_t next();

    /// Class Fields ///

    uint64_t mRand;
    uint64_t mSeed;
};

}

// std::ostream& operator<<( std::ostream& pOS, const hmp::rng_t& pRNG );

#endif
