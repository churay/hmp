#ifndef HMP_CONSTS_H
#define HMP_CONSTS_H

#include "hmp_data.h"

namespace hmp {

LLCE_ACTION_ENUM( lup, ldn, llt, lrt, rup, rdn, rlt, rrt, etc );

LLCE_ENUM( mode, boot_id = -1, exit_id = -2, game_id = 0, title_id, reset_id );
LLCE_ENUM( team, west, east, neutral );

constexpr static int8_t WINNING_SCORE = 3;

};

#endif
