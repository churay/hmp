#include "hmp_rng_t.h"

namespace hmp {

rng_t::rng_t( const uint64_t pSeed) : mRand( pSeed ), mSeed( pSeed ) {
    
}


uint64_t rng_t::next() {
    mRand = ( rng_t::MULTIPLIER * mRand + rng_t::INCREMENT ) % rng_t::PERIOD;
    return mRand;
}

};
