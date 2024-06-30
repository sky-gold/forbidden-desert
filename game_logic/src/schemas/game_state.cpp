#include "game_state.h"
#include <crow/json.h>
#include <vector>

GameState::GameState(){};

GameState::PlayerInfo::PlayerInfo() noexcept {}

GameState::PlayerInfo::PlayerInfo(const crow::json::rvalue &data) {
  user_id = data["user_id"].i();
  type = data["type"].s();
  water_amount = data["water_amount"].i();
  pos = data["pos"].i();
}
crow::json::wvalue GameState::PlayerInfo::as_json() {
  crow::json::wvalue res;
  int water_amount;
  int action_amount;
  int pos_x;
  int pos_y;
  res["user_id"] = user_id;
  res["type"] = type;
  res["water_amount"] = water_amount;
  res["pos"] = pos;
  return res;
}

crow::json::wvalue GameState::GameState::as_json() {
  crow::json::wvalue res;
  res["state_id"] = state_id;
  res["board"] = board;
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
  crow::json::rvalue state_info = crow::json::load(row[2].as<std::string>());
  game_state.player_turn = state_info["player_turn"].i();
  game_state.actions_left = state_info["actions_left"].i();
  std::vector<GameState::PlayerInfo> players_info;
  for (int i = 0; i < state_info["players_info"].size(); ++i) {
    players_info.push_back(
        GameState::PlayerInfo(state_info["players_info"][i]));
  }
  game_state.players_info = players_info;
  std::vector<std::string> board;
  for (int i = 0; i < state_info["board"].size(); ++i) {
    board.push_back(state_info["board"][i].s());
  }
  game_state.board = board;
  return game_state;
}