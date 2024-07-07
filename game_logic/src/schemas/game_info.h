#pragma once

#include <crow/json.h>
#include <pqxx/pqxx>
#include <string>
#include <vector>

class GameInfo {
public:
  GameInfo();

  crow::json::wvalue as_json();

  int id;
  std::string status;
  std::vector<int> players;
  crow::json::rvalue settings;
  std::string created_at;
};

GameInfo readGame(pqxx::work &txn, int id);

void updateGameInfo(pqxx::work &txn, const GameInfo &game_info);