#pragma once

#include "../../schemas/game_info.h"
#include "../../schemas/game_state.h"

std::pair<GameInfo, GameState> startAction(int user_id, int game_id,
                                           const crow::json::rvalue &action,
                                           const GameInfo &game_info);
