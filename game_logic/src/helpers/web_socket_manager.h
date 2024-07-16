#pragma once

#include <crow/websocket.h>
#include <optional>
#include <pqxx/pqxx>
#include <set>
#include <unordered_map>

class WebSocketManager {
public:
  WebSocketManager();
  void open(crow::websocket::connection &conn);
  void close(crow::websocket::connection &conn, const std::string &reason);
  void message(crow::websocket::connection &conn, const std::string &data,
               bool is_binary);
  void game_updated(pqxx::work &txn, int game_id);

private:
  struct ConnectionInfo {
    std::optional<int> game_id;
  };
  std::unordered_map<int, std::set<crow::websocket::connection *>>
      game_connections;
  std::unordered_map<crow::websocket::connection *, ConnectionInfo> connections;
  std::mutex mtx;
};

extern WebSocketManager ws_manager;