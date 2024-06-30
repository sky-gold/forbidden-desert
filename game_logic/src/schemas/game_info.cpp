#include "game_info.h"
#include <crow/json.h>
#include <pqxx/pqxx>
#include <stdexcept>

GameInfo::GameInfo(){};

crow::json::wvalue GameInfo::as_json() {
  crow::json::wvalue res;
  res["id"] = id;
  res["status"] = status;
  res["players"] = players;
  res["settings"] = settings;
  res["created_at"] = created_at;
  return res;
}

GameInfo readGame(pqxx::work &txn, int id) {
  pqxx::result res =
      txn.exec("SELECT * FROM games WHERE id = " + txn.quote(id));
  if (res.size() == 0) {
    throw std::out_of_range("non-existent game");
  }
  pqxx::row row = res[0];
  pqxx::array_parser parser = row["players"].as_array();
  std::vector<int> players;
  std::pair<pqxx::array_parser::juncture, std::string> elem;
  do {
    elem = parser.get_next();
    if (elem.first == pqxx::array_parser::juncture::string_value)
      players.push_back(stoi(elem.second));
  } while (elem.first != pqxx::array_parser::juncture::done);
  GameInfo game;
  game.id = row["id"].as<int>();
  game.status = row["status"].as<std::string>();
  game.players = players;
  game.settings = crow::json::load(row["settings"].as<std::string>());
  game.created_at = row["created_at"].as<std::string>();
  return game;
}