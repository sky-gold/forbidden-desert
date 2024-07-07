#include "leave.h"

#include "../../schemas/game_info.h"
#include "action_checks.h"
#include <vector>

GameInfo leaveAction(int user_id, int game_id, const crow::json::rvalue &action,
                     const GameInfo &game_info) {
  CheckStatus(game_info, "LOBBY");
  CheckInGame(user_id, game_info, true);
  std::vector<int> new_players;
  for (auto to : game_info.players) {
    if (to != user_id) {
      new_players.push_back(to);
    }
  }
  GameInfo new_game_info = game_info;
  new_game_info.players = new_players;
  return new_game_info;
}