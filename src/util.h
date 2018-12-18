#ifndef LLCE_UTIL_H
#define LLCE_UTIL_H

#include "consts.h"

namespace llce {

namespace util {
    /// Namespace Types ///

    struct color_t {
        uint8_t r, g, b, a;
    };

    /// Namespace Functions ///

    // TODO(JRC): Move this function to a better location.
    color_t brighten( const color_t& pColor, const float32_t pFactor );
}

}

#endif
