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
        bool32_t render( const hmp::state_t*, const hmp::input_t*, const hmp::graphics_t* );
    }

    namespace menu {
        bool32_t init( hmp::state_t* pState );
        bool32_t update( hmp::state_t*, hmp::input_t*, const float64_t );
        bool32_t render( const hmp::state_t*, const hmp::input_t*, const hmp::graphics_t* );
    }

    namespace pause {
        bool32_t init( hmp::state_t* pState );
        bool32_t update( hmp::state_t*, hmp::input_t*, const float64_t );
        bool32_t render( const hmp::state_t*, const hmp::input_t*, const hmp::graphics_t* );
    }

    namespace reset {
        bool32_t init( hmp::state_t* pState );
        bool32_t update( hmp::state_t*, hmp::input_t*, const float64_t );
        bool32_t render( const hmp::state_t*, const hmp::input_t*, const hmp::graphics_t* );
    }
}

}

#endif
