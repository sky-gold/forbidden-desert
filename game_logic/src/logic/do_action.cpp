#include "do_action.h"
#include "../schemas/game_action.h"
#include "../schemas/game_info.h"
#include "../schemas/game_state.h"

#include <algorithm>
#include <crow/json.h>
#include <crow/logging.h>
#include <pqxx/pqxx>
#include <stdexcept>
#include <string>
#include <vector>

GameState startState(const GameInfo &game_info) {
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
  return game_state;
}

bool inGame(int user_id, const GameInfo &game_info) {
  return std::find(game_info.players.begin(), game_info.players.end(),
                   user_id) != game_info.players.end();
}

void joinGame(pqxx::work &txn, int user_id, int game_id,
              const crow::json::rvalue &action, const GameInfo &game_info) {
  if (game_info.status != "LOBBY") {
    throw DoActionException("game is not in the LOBBY status, game in " +
                            game_info.status);
  }
  if (game_info.players.size() > 6) {
    throw DoActionException("max amount of players is 6");
  }
  if (inGame(user_id, game_info)) {
    throw DoActionException("user already in game");
  }
  std::string query = "UPDATE games SET players = array_append(players, " +
                      txn.quote(user_id) + ") WHERE id = " + txn.quote(game_id);
  txn.exec(query);
}

void leaveGame(pqxx::work &txn, int user_id, int game_id,
               const crow::json::rvalue &action, const GameInfo &game_info) {
  if (game_info.status != "LOBBY") {
    throw DoActionException("game is not in the LOBBY status, game in " +
                            game_info.status);
  }
  if (!inGame(user_id, game_info)) {
    throw DoActionException("user is not playing this game");
  }
  std::string query = "UPDATE games SET players = array_remove(players, " +
                      txn.quote(user_id) + ") WHERE id = " + txn.quote(game_id);
  txn.exec(query);
}

void startGame(pqxx::work &txn, int user_id, int game_id,
               const crow::json::rvalue &action, const GameInfo &game_info) {
  if (!inGame(user_id, game_info)) {
    throw DoActionException("user is not playing this game");
  }
  if (game_info.status != "LOBBY") {
    throw DoActionException("game is not in the LOBBY status, game in " +
                            game_info.status);
  }
  std::string query = "UPDATE games SET status = 'IN_PROGRESS' WHERE id = " +
                      txn.quote(game_id);
  txn.exec(query);
  GameAction game_action;
  game_action.game_id = game_id;
  game_action.type = "start";
  game_action.info = action;
  game_action.user_id = user_id;
  game_action.action_id = addAction(txn, game_action);
  GameState game_state = startState(game_info);
  game_state.action_id = game_action.action_id;
  addState(txn, game_state);
}

int player_index(int user_id, const GameState &game_state) {
  int i = 0;
  while (i < game_state.players_info.size()) {
    if (game_state.players_info[i].user_id == user_id) {
      break;
    }
    ++i;
  }
  return i;
}

void endOfTurn(pqxx::work &txn, int user_id, int game_id,
               const crow::json::rvalue &action, const GameInfo &game_info,
               const std::vector<GameAction> &actions,
               const GameState &game_state) {
  if (!inGame(user_id, game_info)) {
    throw DoActionException("user is not playing this game");
  }
  if (game_info.status != "IN_PROGRESS") {
    throw DoActionException("game is not in the IN_PROGRESS status");
  }
  if (game_state.player_turn != user_id) {
    throw DoActionException("not player turn");
  }
  GameState new_state = game_state;
  int i = player_index(user_id, game_state);
  new_state.player_turn =
      game_state.players_info[(i + 1) % game_state.players_info.size()].user_id;
  new_state.actions_left = 4;
  GameAction game_action;
  game_action.game_id = game_id;
  game_action.type = "end_of_turn";
  game_action.info = action;
  game_action.user_id = user_id;
  game_action.action_id = addAction(txn, game_action);
  new_state.action_id = game_action.action_id;
  addState(txn, new_state);
  // Вот здесь ещё должна вызываться логика событий
}

void move(pqxx::work &txn, int user_id, int game_id,
          const crow::json::rvalue &action, const GameInfo &game_info,
          const std::vector<GameAction> &actions, const GameState &game_state) {
  if (!inGame(user_id, game_info)) {
    throw DoActionException("user is not playing this game");
  }
  if (game_info.status != "IN_PROGRESS") {
    throw DoActionException("game is not in the IN_PROGRESS status");
  }
  if (game_state.player_turn != user_id) {
    throw DoActionException("not player turn");
  }
  if (game_state.actions_left == 0) {
    throw DoActionException("no actions_left");
  }
  int to = action["to"].i();
  int player_i = player_index(user_id, game_state);
  int pos = game_state.players_info[player_i].pos;
  int dx = abs((to % 5) - (pos % 5));
  int dy = abs((to / 5) - (pos / 5));
  if (dx + dy != 1) {
    CROW_LOG_INFO << "from pos=" << pos << " to=" << to << " dx=" << dx
                  << " dy=" << dy;
    throw DoActionException("illegal move");
  }
  if (game_state.board[pos].sand_amount > 1) {
    throw DoActionException("player is buried in the sand");
  }
  if (game_state.board[to].sand_amount > 1) {
    throw DoActionException("tile is blocked by sand");
  }

  GameState new_state = game_state;
  new_state.players_info[player_i].pos = to;
  new_state.actions_left = game_state.actions_left - 1;
  GameAction game_action;
  game_action.game_id = game_id;
  game_action.type = "move";
  game_action.info = action;
  game_action.user_id = user_id;
  game_action.action_id = addAction(txn, game_action);
  new_state.action_id = game_action.action_id;
  addState(txn, new_state);
}

void dig(pqxx::work &txn, int user_id, int game_id,
         const crow::json::rvalue &action, const GameInfo &game_info,
         const std::vector<GameAction> &actions, const GameState &game_state) {
  if (!inGame(user_id, game_info)) {
    throw DoActionException("user is not playing this game");
  }
  if (game_info.status != "IN_PROGRESS") {
    throw DoActionException("game is not in the IN_PROGRESS status");
  }
  if (game_state.player_turn != user_id) {
    throw DoActionException("not player turn");
  }
  if (game_state.actions_left == 0) {
    throw DoActionException("no actions_left");
  }
  int to = action["to"].i();
  int player_i = player_index(user_id, game_state);
  int pos = game_state.players_info[player_i].pos;
  int dx = abs((to % 5) - (pos % 5));
  int dy = abs((to / 5) - (pos / 5));
  if (dx + dy > 1) {
    CROW_LOG_INFO << "from pos=" << pos << " to=" << to << " dx=" << dx
                  << " dy=" << dy;
    throw DoActionException("tile is too far");
  }
  if (game_state.board[to].sand_amount == 0) {
    throw DoActionException("no sand on tile " + std::to_string(to));
  }
  GameState new_state = game_state;
  new_state.board[to].sand_amount = game_state.board[to].sand_amount - 1;
  new_state.actions_left = game_state.actions_left - 1;
  GameAction game_action;
  game_action.game_id = game_id;
  game_action.type = "dig";
  game_action.info = action;
  game_action.user_id = user_id;
  game_action.action_id = addAction(txn, game_action);
  new_state.action_id = game_action.action_id;
  addState(txn, new_state);
}

void flipOver(pqxx::work &txn, int user_id, int game_id,
              const crow::json::rvalue &action, const GameInfo &game_info,
              const std::vector<GameAction> &actions,
              const GameState &game_state) {
  if (!inGame(user_id, game_info)) {
    throw DoActionException("user is not playing this game");
  }
  if (game_info.status != "IN_PROGRESS") {
    throw DoActionException("game is not in the IN_PROGRESS status");
  }
  if (game_state.player_turn != user_id) {
    throw DoActionException("not player turn");
  }
  if (game_state.actions_left == 0) {
    throw DoActionException("no actions_left");
  }
  int player_i = player_index(user_id, game_state);
  int pos = game_state.players_info[player_i].pos;
  if (game_state.board[pos].is_up) {
    throw DoActionException("tile is already up");
  }
  if (game_state.board[pos].sand_amount) {
    throw DoActionException("tile has sand");
  }
  GameState new_state = game_state;
  new_state.board[pos] = flip(game_state.board, pos);
  new_state.actions_left = game_state.actions_left - 1;
  GameAction game_action;
  game_action.game_id = game_id;
  game_action.type = "dig";
  game_action.info = action;
  game_action.user_id = user_id;
  game_action.action_id = addAction(txn, game_action);
  new_state.action_id = game_action.action_id;
  addState(txn, new_state);
}

void doAction(pqxx::work &txn, int user_id, int game_id,
              const crow::json::rvalue &action) {
  CROW_LOG_INFO << "doAction " << action;
  GameInfo game_info;
  try {
    game_info = readGame(txn, game_id);
  } catch (const std::out_of_range &e) {
    throw DoActionException(e.what());
  }
  if (game_info.status == "FINISHED") {
    throw DoActionException("game is finished");
  }
  if (game_info.status == "CANCELED") {
    throw DoActionException("game is canceled");
  }
  std::string action_type = action["type"].s();
  if (action_type == "join") {
    joinGame(txn, user_id, game_id, action, game_info);
    return;
  }
  if (action_type == "leave") {
    leaveGame(txn, user_id, game_id, action, game_info);
    return;
  }
  if (action_type == "start") {
    startGame(txn, user_id, game_id, action, game_info);
    return;
  }
  std::vector<GameAction> actions = readActions(txn, game_id);
  GameState game_state = readState(txn, actions.back().action_id);
  if (action_type == "end_of_turn") {
    endOfTurn(txn, user_id, game_id, action, game_info, actions, game_state);
    return;
  }
  if (action_type == "move") {
    move(txn, user_id, game_id, action, game_info, actions, game_state);
    return;
  }
  if (action_type == "dig") {
    dig(txn, user_id, game_id, action, game_info, actions, game_state);
    return;
  }
  if (action_type == "flip_over") {
    flipOver(txn, user_id, game_id, action, game_info, actions, game_state);
    return;
  }
  throw DoActionException("Not implemented action: " + action_type);
}