#include "web_socket_manager.h"
#include "full_game.h"

#include <crow/json.h>
#include <crow/logging.h>
#include <exception>
#include <mutex>

WebSocketManager::WebSocketManager() = default;

void WebSocketManager::open(crow::websocket::connection &conn) {
  std::lock_guard<std::mutex> _(mtx);
  CROW_LOG_INFO << "new websocket connection from " << conn.get_remote_ip();
  connections.insert({&conn, WebSocketManager::ConnectionInfo()});
}

void WebSocketManager::close(crow::websocket::connection &conn,
                             const std::string &reason) {
  CROW_LOG_INFO << "websocket connection closed: " << reason;
  std::lock_guard<std::mutex> _(mtx);
  connections.erase(&conn);
}

void WebSocketManager::message(crow::websocket::connection &conn,
                               const std::string &data, bool is_binary) {
  CROW_LOG_INFO << "new websocket message from " << conn.get_remote_ip();
  std::lock_guard<std::mutex> _(mtx);
  if (is_binary) {
    CROW_LOG_ERROR << "message is binary not implemented";
    return;
  }
  int game_id;
  try {
    game_id = std::stoi(data);
  } catch (const std::exception &e) {
    CROW_LOG_ERROR << "can't convert message to game_id: " << e.what();
  }
  if (connections[&conn].game_id.has_value()) {
    game_connections[connections[&conn].game_id.value()].erase(&conn);
  }
  game_connections[game_id].insert(&conn);
  connections[&conn].game_id = game_id;
  CROW_LOG_INFO << "connection " << conn.get_remote_ip()
                << " now watching game " << game_id;
}

void WebSocketManager::game_updated(pqxx::work &txn, int game_id) {
  CROW_LOG_INFO << "WebSocketManager game updated";
  if (!game_connections.count(game_id)) {
    CROW_LOG_INFO << "No connections for this game";
    return;
  }
  crow::json::wvalue info;
  try {
    info = readFullGameInfo(txn, game_id, false);
  } catch (const std::exception &e) {
    CROW_LOG_ERROR << "Failed to read game:" << e.what();
    return;
  }
  for (auto conn : game_connections[game_id]) {
    try {
      conn->send_text(info.dump());
    } catch (const std::exception &e) {
      CROW_LOG_ERROR << "Failed to send info:" << e.what();
      return;
    }
  }
}

WebSocketManager ws_manager;