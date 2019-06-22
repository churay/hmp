#ifndef HMP_MODES_T_H
#define HMP_MODES_T_H

#include "hmp.h"
#include "hmp_consts.h"
#include "consts.h"

namespace hmp {

namespace mode {
    namespace game {
        void init( hmp::state_t* pState );
        void update( hmp::state_t*, hmp::input_t*, const float64_t );
        void render( const hmp::state_t*, const hmp::input_t*, const hmp::graphics_t* );
    }

    namespace menu {
        void init( hmp::state_t* pState );
        void update( hmp::state_t*, hmp::input_t*, const float64_t );
        void render( const hmp::state_t*, const hmp::input_t*, const hmp::graphics_t* );
    }

    namespace pause {
        void init( hmp::state_t* pState );
        void update( hmp::state_t*, hmp::input_t*, const float64_t );
        void render( const hmp::state_t*, const hmp::input_t*, const hmp::graphics_t* );
    }

    namespace reset {
        void init( hmp::state_t* pState );
        void update( hmp::state_t*, hmp::input_t*, const float64_t );
        void render( const hmp::state_t*, const hmp::input_t*, const hmp::graphics_t* );
    }
}

}

#endif
