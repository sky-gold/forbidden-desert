#include "../helpers/connection_pool.h"
#include "../logic/action_validate.h"
#include "../logic/do_action.h"
#include "../middlewares/auth.h"
#include <crow/app.h>
#include <crow/http_request.h>
#include <crow/http_response.h>
#include <crow/json.h>
#include <crow/logging.h>
#include <pqxx/pqxx>
#include <stdexcept>

template <typename... Middlewares>
crow::response actionHandler(crow::App<Middlewares...> &app,
                             const crow::request &req, int game_id) {
  CROW_LOG_INFO << "actionHandler";
  int user_id = app.template get_context<AuthGuard>(req).id;
  auto connection = makeScopeGuard(pg_pool->connection(),
                                   [](std::shared_ptr<pqxx::connection> conn) {
                                     pg_pool->freeConnection(conn);
                                   });
  crow::json::rvalue action = crow::json::load(req.body);
  pqxx::work txn(*(connection.get()));
  txn.exec("SET TRANSACTION ISOLATION LEVEL SERIALIZABLE");
  try {
    // Check that action is valid (has need fields)
    if (!actionValidate(action)) {
      return crow::response(400, "invalid action");
    }
    // Do action (can throw exception if action is incorrect)
    try {
      doAction(txn, user_id, game_id, action);
    } catch (const std::invalid_argument &e) {
      return crow::response(400, e.what());
    }
    txn.commit();
  } catch (const std::exception &e) {
    CROW_LOG_ERROR << "Transaction failed: " << e.what();
    return crow::response(500);
  }
  return crow::response(200, "OK");
}