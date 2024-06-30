#pragma once

#include <crow/json.h>
#include <pqxx/pqxx>
#include <vector>

class GameAction {
public:
  GameAction();

  crow::json::wvalue as_json();

  int action_id;
  int game_id;
  int user_id;
  std::string type;
  crow::json::rvalue info;
};

std::vector<GameAction> readActions(pqxx::work &txn, int game_id);