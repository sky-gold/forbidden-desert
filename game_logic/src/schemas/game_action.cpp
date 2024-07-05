#include "game_action.h"
#include <crow/json.h>
#include <crow/logging.h>
#include <string>

GameAction::GameAction(){};

crow::json::wvalue GameAction::as_json() {
  crow::json::wvalue res;
  res["action_id"] = action_id;
  res["game_id"] = game_id;
  res["user_id"] = user_id;
  res["type"] = type;
  res["info"] = crow::json::wvalue(info);
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
    actions.emplace_back(std::move(action));
  }
  return actions;
}

int addAction(pqxx::work &txn, const GameAction &action) {
  pqxx::result r =
      txn.exec("SELECT MAX(action_number) FROM actions WHERE game_id = " +
               std::to_string(action.game_id));
  int action_number = 1;
  if (!r[0][0].is_null()) {
    action_number = r[0][0].as<int>() + 1;
  }
  std::string query_insert_action =
      "INSERT INTO actions (game_id, user_id, info, type, action_number) "
      "VALUES (" +
      std::to_string(action.game_id) + ", " + std::to_string(action.user_id) +
      ", " + txn.quote(crow::json::wvalue(action.info).dump()) + ", " +
      txn.quote(action.type) + ", " + std::to_string(action_number) +
      ") RETURNING action_id";

  pqxx::result result = txn.exec(query_insert_action);
  return result[0][0].as<int>();
}