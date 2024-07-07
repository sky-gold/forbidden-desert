#pragma once

#include "../../schemas/game_info.h"
#include "../../schemas/game_state.h"

GameState flipOverAction(int user_id, int game_id,
                         const crow::json::rvalue &action,
                         const GameInfo &game_info,
                         const GameState &game_state);