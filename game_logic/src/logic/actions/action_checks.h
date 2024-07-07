#pragma once

#include "../../schemas/game_info.h"
#include "../../schemas/game_state.h"

void CheckInGame(int user_id, const GameInfo &game_info, bool expected_in);

void CheckStatus(const GameInfo &game_info, const std::string &expected_status);

void CheckTurn(int user_id, const GameState &game_state);

void ActionsLeft(const GameState &game_state);