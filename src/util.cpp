#include <cmath>

#include "util.h"

namespace llce {

util::color_t util::brighten( const util::color_t& pColor, const float32_t pFactor ) {
    return {
        (uint8_t)std::min( (int32_t)255, (int32_t)(pFactor * pColor.r) ),
        (uint8_t)std::min( (int32_t)255, (int32_t)(pFactor * pColor.g) ),
        (uint8_t)std::min( (int32_t)255, (int32_t)(pFactor * pColor.b) ),
        pColor.a
    };
}

}
