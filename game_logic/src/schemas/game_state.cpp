#include "game_state.h"
#include <crow/json.h>
#include <set>
#include <stdexcept>
#include <vector>

GameState::GameState(){};

GameState::PlayerInfo::PlayerInfo() noexcept {}

GameState::TileInfo::TileInfo() noexcept {};

GameState::PlayerInfo::PlayerInfo(const crow::json::rvalue &data) {
  user_id = data["user_id"].i();
  type = data["type"].s();
  water_amount = data["water_amount"].i();
  pos = data["pos"].i();
}

GameState::TileInfo::TileInfo(const crow::json::rvalue &data) {
  name = data["name"].s();
  sand_amount = data["sand_amount"].i();
  is_up = data["is_up"].b();
}

crow::json::wvalue GameState::PlayerInfo::as_json() {
  crow::json::wvalue res;
  res["user_id"] = user_id;
  res["type"] = type;
  res["water_amount"] = water_amount;
  res["pos"] = pos;
  return res;
}

crow::json::wvalue GameState::TileInfo::as_json() {
  crow::json::wvalue res;
  res["name"] = name;
  res["sand_amount"] = sand_amount;
  res["is_up"] = is_up;
  return res;
}

crow::json::wvalue GameState::GameState::as_json() const {
  crow::json::wvalue res;
  res["state_id"] = state_id;
  res["action_id"] = action_id;
  std::vector<crow::json::wvalue> board_json;
  for (auto tile : board) {
    board_json.push_back(tile.as_json());
  }
  res["board"] = crow::json::wvalue::list(board_json);
  res["player_turn"] = player_turn;
  res["actions_left"] = actions_left;
  std::vector<crow::json::wvalue> players_info_json;
  for (auto player : players_info) {
    players_info_json.push_back(player.as_json());
  }
  res["players_info"] = crow::json::wvalue::list(players_info_json);
  return res;
}

GameState readState(pqxx::work &txn, int action_id) {
  std::string query = "SELECT state_id, action_id, state_info FROM game_states "
                      "WHERE action_id = " +
                      txn.quote(action_id) + " LIMIT 1";
  pqxx::result res(txn.exec(query));

  if (res.empty()) {
    throw std::out_of_range("non-existent state for action_id");
  }
  auto row = res[0];
  GameState game_state;
  game_state.state_id = row[0].as<int>();
  game_state.action_id = row[1].as<int>();
  crow::json::rvalue state_info = crow::json::load(row[2].as<std::string>());
  game_state.player_turn = state_info["player_turn"].i();
  game_state.actions_left = state_info["actions_left"].i();
  std::vector<GameState::PlayerInfo> players_info;
  for (int i = 0; i < state_info["players_info"].size(); ++i) {
    players_info.push_back(
        GameState::PlayerInfo(state_info["players_info"][i]));
  }
  game_state.players_info = players_info;
  std::vector<GameState::TileInfo> board;
  for (int i = 0; i < state_info["board"].size(); ++i) {
    board.push_back(GameState::TileInfo(state_info["board"][i]));
  }
  game_state.board = board;
  return game_state;
}

std::vector<GameState::TileInfo> generateStartBoard() {
  std::vector<GameState::TileInfo> board(25);
  for (int i = 0; i < 25; ++i) {
    board[i].name = "tile_down";
  }
  board[0].name = "tile_down_start";
  board[12].name = "storm";
  for (int i = 0; i < 25; ++i) {
    int dx = abs((i % 5) - 2);
    int dy = abs((i / 5) - 2);
    if (dy + dx == 2) {
      board[i].sand_amount = 1;
    }
  }
  return board;
}

int addState(pqxx::work &txn, const GameState &game_state) {
  std::string query_insert_state =
      "INSERT INTO game_states (action_id, state_info) VALUES (" +
      std::to_string(game_state.action_id) + ", " +
      txn.quote(game_state.as_json().dump()) + ") RETURNING state_id";
  pqxx::result result = txn.exec(query_insert_state);
  return result[0][0].as<int>();
}

const std::map<std::string, std::vector<std::string>> BOARD_FLIP_OVER = {
    {"tile_down_start", {"start"}},
    {"tile_down_water", {"water_1", "water_2", "mirage"}},
    {"tile_down",
     {"1",  "2",  "3",  "4",  "5",  "6",  "7",  "8",  "9",  "10",
      "11", "12", "13", "14", "15", "16", "17", "18", "19", "launch_pad"}},
};

GameState::TileInfo flip(const std::vector<GameState::TileInfo> &board,
                         int index) {
  if (board[index].is_up) {
    throw std::invalid_argument("can't flip: tile is up");
  }
  if (board[index].sand_amount > 0) {
    throw std::invalid_argument("can't flip: tile has sand");
  }
  std::string tile_name = board[index].name;
  std::set<std::string> possible_answers;
  for (auto name : BOARD_FLIP_OVER.at(tile_name)) {
    possible_answers.insert(name);
  }
  for (const auto &tile : board) {
    possible_answers.erase(tile.name);
  }
  if (possible_answers.empty()) {
    throw std::runtime_error("can't flip: possible_answers is empty");
  }
  int r = std::rand() % possible_answers.size();
  auto it = std::begin(possible_answers);
  std::advance(it, r);
  std::string new_name = *it;
  if (index == 1 && possible_answers.count("launch_pad")) {
    new_name = "launch_pad";
  }
  GameState::TileInfo tile;
  tile.name = new_name;
  tile.is_up = true;
  tile.sand_amount = 0;
  return tile;
}