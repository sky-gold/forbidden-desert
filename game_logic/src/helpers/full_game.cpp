#include "full_game.h"
#include "../schemas/game_action.h"
#include "../schemas/game_info.h"
#include "../schemas/game_state.h"

crow::json::wvalue readFullGameInfo(pqxx::work &txn, int game_id,
                                    bool return_actions) {
  crow::json::wvalue result;
  GameInfo game_info = readGame(txn, game_id);
  result["game_info"] = game_info.as_json();
  std::vector<GameAction> actions = readActions(txn, game_id);
  if (return_actions) {
    std::vector<crow::json::wvalue> actions_json;
    for (auto action : actions) {
      actions_json.push_back(action.as_json());
    }
    result["actions"] = crow::json::wvalue::list(actions_json);
  }
  if (actions.empty()) {
    result["game_state"] = crow::json::load("{}");
  } else {
    GameState game_state = readState(txn, actions.back().action_id);
    result["game_state"] = game_state.as_json();
  }
  return result;
}