#include "rng_t.h"

namespace llce {

rng_t::rng_t( const uint64_t pSeed ) : mRand( pSeed ), mSeed( pSeed ) {
    
}


uint64_t rng_t::next() {
    mRand = ( rng_t::MULTIPLIER * mRand + rng_t::INCREMENT ) % rng_t::PERIOD;
    return mRand;
}


float64_t rng_t::nextf() {
    return ( next() % (1 << 16) ) / ( (1 << 16) + 0.0 );
}

};
