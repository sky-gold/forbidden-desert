#include "join.h"

#include "../../schemas/game_info.h"
#include "../do_action_exception.h"
#include "action_checks.h"

GameInfo joinAction(int user_id, int game_id, const crow::json::rvalue &action,
                    const GameInfo &game_info) {
  CheckStatus(game_info, "LOBBY");
  CheckInGame(user_id, game_info, false);
  if (game_info.players.size() >= 6) {
    throw DoActionException("max amount of players is 6");
  }
  GameInfo new_game_info = game_info;
  new_game_info.players.push_back(user_id);
  return new_game_info;
}