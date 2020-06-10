#ifndef HMP_CONSTS_H
#define HMP_CONSTS_H

#include "hmp_data.h"

namespace hmp {

namespace action { enum action_e { unbound = 0, lup, ldn, llt, lrt, rup, rdn, rlt, rrt, etc, _length }; };
typedef action::action_e action_e;

namespace mode { enum mode_e { boot_id = -1, exit_id = -2, game_id = 0, title_id, reset_id }; };
typedef mode::mode_e mode_e;

namespace team { enum team_e { west, east, neutral }; };
typedef team::team_e team_e;

constexpr static int8_t WINNING_SCORE = 3;

};

#endif
