#ifndef LLCE_CLI_H
#define LLCE_CLI_H

#include "consts.h"

namespace llce {

namespace cli {
    /// Namespace Functions ///

    bool8_t exists( const char8_t* pOption, const char8_t** pArgs, const int32_t pArgCount );
    const char8_t* value( const char8_t* pOption, const char8_t** pArgs, const int32_t pArgCount );
}

}

#endif
