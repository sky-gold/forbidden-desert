
#include <algorithm>

#include "../../schemas/game_info.h"
#include "../../schemas/game_state.h"
#include "../do_action_exception.h"

void CheckInGame(int user_id, const GameInfo &game_info, bool expected_in) {
  if (std::find(game_info.players.begin(), game_info.players.end(), user_id) !=
      game_info.players.end()) {
    if (!expected_in) {
      throw DoActionException("user already in game");
    }
  } else {
    if (expected_in) {
      throw DoActionException("user is not playing this game");
    }
  }
}

void CheckStatus(const GameInfo &game_info,
                 const std::string &expected_status) {
  if (game_info.status != expected_status) {
    throw DoActionException("game wrong status, expected " + expected_status +
                            " get " + game_info.status);
  }
}

void CheckTurn(int user_id, const GameState &game_state) {
  if (game_state.player_turn != user_id) {
    throw DoActionException("not player turn");
  }
}

void ActionsLeft(const GameState &game_state) {
  if (game_state.actions_left == 0) {
    throw DoActionException("no actions_left");
  }
}