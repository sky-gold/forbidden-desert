#include "game_action.h"

GameAction::GameAction(){};

crow::json::wvalue GameAction::as_json() {
  crow::json::wvalue res;
  res["action_id"] = action_id;
  res["game_id"] = game_id;
  res["user_id"] = user_id;
  res["type"] = type;
  res["info"] = info;
  return res;
}

std::vector<GameAction> readActions(pqxx::work &txn, int game_id) {
  std::string query =
      "SELECT action_id, game_id, action_number, user_id, info, type "
      "FROM actions WHERE game_id = " +
      std::to_string(game_id) + " ORDER BY action_number";
  pqxx::result R(txn.exec(query));
  std::vector<GameAction> actions;
  for (pqxx::result::const_iterator c = R.begin(); c != R.end(); ++c) {
    GameAction action;
    action.action_id = c[0].as<int>();
    action.game_id = c[1].as<int>();
    action.user_id = c[3].as<int>();
    action.info = crow::json::load(c[4].as<std::string>());
    action.type = c[5].as<std::string>();
    actions.push_back(action);
  }
  return actions;
}