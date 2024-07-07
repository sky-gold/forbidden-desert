#include "start.h"

#include "../../schemas/game_info.h"
#include "../../schemas/game_state.h"
#include "action_checks.h"

std::pair<GameInfo, GameState> startAction(int user_id, int game_id,
                                           const crow::json::rvalue &action,
                                           const GameInfo &game_info) {
  CheckInGame(user_id, game_info, true);
  CheckStatus(game_info, "LOBBY");
  GameInfo new_game_info = game_info;
  new_game_info.status = "IN_PROGRESS";
  GameState game_state;
  std::vector<std::string> types = {"red",    "blue",  "green",
                                    "yellow", "black", "white"};
  std::random_shuffle(types.begin(), types.end());
  for (int player_id : game_info.players) {
    GameState::PlayerInfo player_info;
    player_info.pos = 0;
    player_info.type = types.back();
    types.pop_back();
    player_info.user_id = player_id;
    player_info.water_amount = 5;
    game_state.players_info.push_back(player_info);
  }
  game_state.player_turn = game_state.players_info[0].user_id;
  game_state.actions_left = 4;
  game_state.board = generateStartBoard();
  return {new_game_info, game_state};
}