#include "dig.h"

#include "../../schemas/game_info.h"
#include "../../schemas/game_state.h"
#include "../do_action_exception.h"
#include "action_checks.h"
#include <string>

GameState digAction(int user_id, int game_id, const crow::json::rvalue &action,
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
  if (dx + dy > 1) {
    throw DoActionException("illegal dig dx=" + std::to_string(dx) +
                            " dy=" + std::to_string(dy));
  }
  if (game_state.board[to].sand_amount == 0) {
    throw DoActionException("no sand on tile " + std::to_string(to));
  }
  GameState new_state = game_state;
  new_state.board[to].sand_amount = game_state.board[to].sand_amount - 1;
  new_state.actions_left = game_state.actions_left - 1;
  return new_state;
}