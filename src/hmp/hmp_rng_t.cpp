#include "hmp_rng_t.h"

namespace hmp {

rng_t::rng_t( const uint64_t pSeed) : mSeed( pSeed ) {
    
}


uint64_t rng_t::next() {
    return 1;
}

};
