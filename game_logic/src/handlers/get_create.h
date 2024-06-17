#include "../helpers/connection_pool.h"
#include "../middlewares/auth.h"
#include <crow/app.h>
#include <crow/http_request.h>
#include <crow/http_response.h>
#include <crow/logging.h>

template <typename... Middlewares>
crow::response createHandler(crow::App<Middlewares...> &app,
                             const crow::request &req) {
  CROW_LOG_INFO << "createHandler";
  int user_id = app.template get_context<AuthGuard>(req).id;
  CROW_LOG_INFO << "user_id=" << user_id;
  auto connection = pg_pool->connection();
  try {
    pqxx::work txn(*connection);
    pqxx::result res = txn.exec("SELECT VERSION()");
    std::ostringstream oss;
    for (const auto &row : res) {
      for (const auto &field : row) {
        oss << field.c_str() << "\t";
      }
      oss << "\n";
    }
    oss.str();
    return crow::response(200, oss.str());
  } catch (const std::exception &e) {
    CROW_LOG_ERROR << e.what();
    pg_pool->freeConnection(connection);
    return crow::response(500);
  }
}