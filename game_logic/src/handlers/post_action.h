#include "../helpers/connection_pool.h"
#include "../helpers/web_socket_manager.h"
#include "../logic/action_validate.h"
#include "../logic/do_action.h"
#include "../logic/do_action_exception.h"
#include "../logic/end_game.h"
#include "../middlewares/auth.h"
#include <crow/app.h>
#include <crow/http_request.h>
#include <crow/http_response.h>
#include <crow/json.h>
#include <crow/logging.h>
#include <pqxx/pqxx>

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
    try {
      actionValidate(action);
    } catch (const ActionValidationException &e) {
      return crow::response(400, e.what());
    }
    // Do action (can throw exception if action is incorrect)
    try {
      doAction(txn, user_id, game_id, action);
    } catch (const DoActionException &e) {
      return crow::response(400, e.what());
    }
    TryToEnd(txn, game_id);
    ws_manager.game_updated(txn, game_id);
    txn.commit();
  } catch (const std::exception &e) {
    CROW_LOG_ERROR << "Transaction failed: " << e.what();
    return crow::response(500);
  }
  return crow::response(200, "OK");
}