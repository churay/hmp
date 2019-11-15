#ifndef LLCE_METAVIS_H
#define LLCE_METAVIS_H

#include "hmp/hmp.h"
#include "consts.h"

namespace llce {

namespace meta {

bool32_t update( const hmp::state_t* pState, const hmp::input_t* pInput, const hmp::output_t* pOutput, const float64_t pDT );
bool32_t render( const hmp::state_t* pState, const hmp::input_t* pInput, const hmp::output_t* pOutput );

}

}

#endif
