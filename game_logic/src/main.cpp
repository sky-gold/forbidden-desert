#include "handlers/get_create.h"
#include "handlers/get_info.h"
#include "handlers/post_action.h"
#include "helpers/connection_pool.h"
#include "helpers/env_vars.h"
#include "helpers/web_socket_manager.h"
#include "middlewares/auth.h"

#include <crow.h>
#include <crow/app.h>
#include <crow/http_request.h>
#include <crow/http_response.h>
#include <crow/logging.h>
#include <crow/middleware_context.h>
#include <crow/routing.h>
#include <string>

int main() {
  EnvVars::init();
  db_init();

  crow::App<AuthGuard> app;

  CROW_WEBSOCKET_ROUTE(app, "/ws")
      .onopen([&](crow::websocket::connection &conn) { ws_manager.open(conn); })
      .onclose(
          [&](crow::websocket::connection &conn, const std::string &reason) {
            ws_manager.close(conn, reason);
          })
      .onmessage(
          [&](crow::websocket::connection &conn, const std::string &data,
              bool is_binary) { ws_manager.message(conn, data, is_binary); });

  CROW_ROUTE(app, "/create")
  ([&app](const crow::request &req) { return createHandler(app, req); });

  CROW_ROUTE(app, "/info/<int>")
  ([&app](int game_id) { return infoHandler(app, game_id); });

  CROW_ROUTE(app, "/action/<int>")
      .methods("POST"_method)([&app](const crow::request &req, const int &id) {
        try {
          return actionHandler(app, req, id);
        } catch (const std::exception &e) {
          CROW_LOG_ERROR << e.what();
          return crow::response(500);
        }
      });
  app.port(18080).multithreaded().run();
  return 0;
}