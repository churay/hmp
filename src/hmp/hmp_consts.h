#ifndef HMP_CONSTS_H
#define HMP_CONSTS_H

#include "hmp_data.h"

namespace hmp {

namespace mode { enum mode_e { boot_id = -1, exit_id = -2, game_id = 0, menu_id, reset_id }; };
namespace team { enum team_e { west, east, neutral }; };

constexpr static int8_t WINNING_SCORE = 3;

};

#endif
