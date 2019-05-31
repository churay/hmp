#include <cmath>
#include <cstring>

#include "cli.h"

namespace llce {

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
