#pragma once

#include <crow/json.h>
#include <pqxx/pqxx>
#include <string>
#include <vector>

class GameState {
public:
  struct PlayerInfo {
    PlayerInfo() noexcept;
    PlayerInfo(const crow::json::rvalue &data);
    int user_id;
    std::string type;
    int water_amount;
    int pos;
    crow::json::wvalue as_json();
  };

  GameState();

  crow::json::wvalue as_json();

  int state_id;
  std::vector<PlayerInfo> players_info;
  std::vector<std::string> board;
  int player_turn;
  int actions_left;
};

GameState readState(pqxx::work &txn, int action_id);