#include "do_action.h"
#include "../schemas/game_info.h"

#include <crow/json.h>
#include <crow/logging.h>
#include <pqxx/pqxx>
#include <stdexcept>
#include <string>

void joinGame(pqxx::work &txn, int user_id, int game_id,
              const crow::json::rvalue &action, const GameInfo &game_info) {
  if (game_info.players.size() > 6) {
    throw std::invalid_argument("max amount of players is 6");
  }
  if (std::find(game_info.players.begin(), game_info.players.end(), user_id) !=
      game_info.players.end()) {
    throw std::invalid_argument("user already in game");
  }
  std::string query = "UPDATE games SET players = array_append(players, " +
                      txn.quote(user_id) + ") WHERE id = " + txn.quote(game_id);
  txn.exec(query);
}

void leaveGame(pqxx::work &txn, int user_id, int game_id,
               const crow::json::rvalue &action, const GameInfo &game_info) {
  if (std::find(game_info.players.begin(), game_info.players.end(), user_id) ==
      game_info.players.end()) {
    throw std::invalid_argument("user is not playing this game");
  }
  std::string query = "UPDATE games SET players = array_remove(players, " +
                      txn.quote(user_id) + ") WHERE id = " + txn.quote(game_id);
  txn.exec(query);
}

void doAction(pqxx::work &txn, int user_id, int game_id,
              const crow::json::rvalue &action) {
  CROW_LOG_INFO << "doAction " << action;
  GameInfo game_info;
  try {
    game_info = readGame(txn, game_id);
  } catch (const std::out_of_range &e) {
    throw std::invalid_argument(e.what());
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
}