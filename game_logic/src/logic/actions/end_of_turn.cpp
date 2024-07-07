#include "end_of_turn.h"

#include "../../schemas/game_info.h"
#include "../../schemas/game_state.h"
#include "action_checks.h"

GameState endOfTurnAction(int user_id, int game_id,
                          const crow::json::rvalue &action,
                          const GameInfo &game_info,
                          const GameState &game_state) {
  CheckInGame(user_id, game_info, true);
  CheckStatus(game_info, "IN_PROGRESS");
  CheckTurn(user_id, game_state);
  GameState new_state = game_state;
  int i = game_state.player_index(user_id);
  new_state.player_turn =
      game_state.players_info[(i + 1) % game_state.players_info.size()].user_id;
  new_state.actions_left = 4;
  // После этого ещё должны карты бури вызываться
  return new_state;
}