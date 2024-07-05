#include "end_game.h"
#include "../schemas/game_action.h"
#include "../schemas/game_info.h"
#include "../schemas/game_state.h"

#include <pqxx/pqxx>

void TryToEnd(pqxx::work &txn, int game_id) {
  GameInfo game_info = readGame(txn, game_id);
  if (game_info.status != "IN_PROGRESS") {
    return;
  }
  std::vector<GameAction> actions = readActions(txn, game_id);
  GameState game_state = readState(txn, actions.back().action_id);
  int launch_pad = -1;
  for (int i = 0; i < 25; ++i) {
    if (game_state.board[i].name == "launch_pad") {
      launch_pad = i;
    }
  }
  bool all_players_at_launch_pad = true;
  for (auto player_info : game_state.players_info) {
    if (player_info.pos != launch_pad) {
      all_players_at_launch_pad = false;
    }
  }
  if (all_players_at_launch_pad) {
    std::string query =
        "UPDATE games SET status = 'FINISHED' WHERE id = " + txn.quote(game_id);
    txn.exec(query);
  }
}