#pragma once

#include "../../schemas/game_info.h"

GameInfo joinAction(int user_id, int game_id, const crow::json::rvalue &action,
                    const GameInfo &game_info);
