#include "move.h"

#include "../../schemas/game_info.h"
#include "../../schemas/game_state.h"
#include "../do_action_exception.h"
#include "action_checks.h"
#include <string>

GameState moveAction(int user_id, int game_id, const crow::json::rvalue &action,
                     const GameInfo &game_info, const GameState &game_state) {
  CheckInGame(user_id, game_info, true);
  CheckStatus(game_info, "IN_PROGRESS");
  CheckTurn(user_id, game_state);
  ActionsLeft(game_state);
  int to = action["to"].i();
  int player_i = game_state.player_index(user_id);
  int pos = game_state.players_info[player_i].pos;
  int dx = abs((to % 5) - (pos % 5));
  int dy = abs((to / 5) - (pos / 5));
  if (dx + dy != 1) {
    throw DoActionException("illegal move dx=" + std::to_string(dx) +
                            " dy=" + std::to_string(dy));
  }
  if (game_state.board[pos].sand_amount > 1) {
    throw DoActionException("player is buried in the sand");
  }
  if (game_state.board[to].sand_amount > 1) {
    throw DoActionException("tile is blocked by sand");
  }

  GameState new_state = game_state;
  new_state.players_info[player_i].pos = to;
  new_state.actions_left = game_state.actions_left - 1;
  return new_state;
}