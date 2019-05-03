#include <cmath>
#include <cstring>

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


bool8_t cli::exists( const char8_t* pOption, const char8_t** pArgs, const int32_t pArgCount ) {
    bool8_t optionFound = false;

    for( int32_t argIdx = 0; argIdx < pArgCount; argIdx++ ) {
        const char8_t* arg = pArgs[argIdx];
        optionFound |= ( strcmp(pOption, arg) == 0 );
    }

    return optionFound;
}


const char8_t* cli::value( const char8_t* pOption, const char8_t** pArgs, const int32_t pArgCount ) {
    const char8_t* optionValue = nullptr;

    for( int32_t argIdx = 0; argIdx < pArgCount - 1; argIdx++ ) {
        const char8_t* arg = pArgs[argIdx];
        if( strcmp(pOption, arg) == 0 ) {
            optionValue = pArgs[argIdx + 1];
        }
    }

    return optionValue;
}

}
