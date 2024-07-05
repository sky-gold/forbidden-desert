#pragma once

#include "game_info.h"
#include <crow/json.h>
#include <pqxx/pqxx>
#include <string>
#include <vector>

class GameState {
public:
  struct PlayerInfo {
    PlayerInfo() noexcept;
    PlayerInfo(const crow::json::rvalue &data);
    crow::json::wvalue as_json();
    int user_id;
    std::string type;
    int water_amount;
    int pos;
  };

  struct TileInfo {
    TileInfo() noexcept;
    TileInfo(const crow::json::rvalue &data);
    crow::json::wvalue as_json();
    std::string name;
    bool is_up = false;
    int sand_amount = 0;
  };

  GameState();

  crow::json::wvalue as_json() const;

  int state_id;
  int action_id;
  std::vector<PlayerInfo> players_info;
  std::vector<TileInfo> board;
  int player_turn;
  int actions_left;
};

GameState readState(pqxx::work &txn, int action_id);

extern const std::map<std::string, std::vector<std::string>> BOARD_FLIP_OVER;

std::vector<GameState::TileInfo> generateStartBoard();

int addState(pqxx::work &txn, const GameState &game_state);

GameState::TileInfo flip(const std::vector<GameState::TileInfo> &board,
                         int index);