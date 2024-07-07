#include "flip_over.h"

#include "../../schemas/game_info.h"
#include "../../schemas/game_state.h"
#include "../do_action_exception.h"
#include "action_checks.h"
#include <string>

GameState flipOverAction(int user_id, int game_id,
                         const crow::json::rvalue &action,
                         const GameInfo &game_info,
                         const GameState &game_state) {
  CheckInGame(user_id, game_info, true);
  CheckStatus(game_info, "IN_PROGRESS");
  CheckTurn(user_id, game_state);
  ActionsLeft(game_state);
  int player_i = game_state.player_index(user_id);
  int pos = game_state.players_info[player_i].pos;
  if (game_state.board[pos].is_up) {
    throw DoActionException("tile is already up");
  }
  if (game_state.board[pos].sand_amount) {
    throw DoActionException("tile has sand");
  }
  GameState new_state = game_state;
  new_state.board[pos] = flip(game_state.board, pos);
  new_state.actions_left = game_state.actions_left - 1;
  return new_state;
}