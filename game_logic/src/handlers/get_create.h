#include "../helpers/connection_pool.h"
#include "../middlewares/auth.h"
#include <crow/app.h>
#include <crow/http_request.h>
#include <crow/http_response.h>
#include <crow/logging.h>
#include <string>

template <typename... Middlewares>
crow::response createHandler(crow::App<Middlewares...> &app,
                             const crow::request &req) {
  int user_id = app.template get_context<AuthGuard>(req).id;
  auto connection = makeScopeGuard(pg_pool->connection(),
                                   [](std::shared_ptr<pqxx::connection> conn) {
                                     pg_pool->freeConnection(conn);
                                   });
  pqxx::work txn(*(connection.get()));
  std::string settings_str = "{\"difficulty\": \"easy\"}";
  std::string query = "INSERT INTO games (status, players, settings) VALUES "
                      "('LOBBY', ARRAY[" +
                      std::to_string(user_id) + "], '" + txn.esc(settings_str) +
                      "') RETURNING id";
  pqxx::result r = txn.exec(query);
  int game_id = r[0][0].as<int>();
  txn.commit();
  return crow::response(200, std::to_string(game_id));
}