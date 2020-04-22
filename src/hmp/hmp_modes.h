#ifndef HMP_MODES_T_H
#define HMP_MODES_T_H

#include "hmp.h"
#include "hmp_consts.h"
#include "consts.h"

namespace hmp {

namespace mode {
    namespace game {
        bool32_t init( hmp::state_t* pState );
        bool32_t update( hmp::state_t*, hmp::input_t*, const float64_t );
        bool32_t render( const hmp::state_t*, const hmp::input_t*, const hmp::output_t* );
    }

    namespace title {
        bool32_t init( hmp::state_t* pState );
        bool32_t update( hmp::state_t*, hmp::input_t*, const float64_t );
        bool32_t render( const hmp::state_t*, const hmp::input_t*, const hmp::output_t* );
    }

    namespace reset {
        bool32_t init( hmp::state_t* pState );
        bool32_t update( hmp::state_t*, hmp::input_t*, const float64_t );
        bool32_t render( const hmp::state_t*, const hmp::input_t*, const hmp::output_t* );
    }
}

}

#endif
