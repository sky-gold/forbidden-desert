#include "../helpers/connection_pool.h"
#include "../helpers/scope_guard.h"
#include "../schemas/game_action.h"
#include "../schemas/game_info.h"
#include "../schemas/game_state.h"

#include <crow/app.h>
#include <crow/http_request.h>
#include <crow/http_response.h>
#include <crow/json.h>
#include <crow/logging.h>
#include <stdexcept>
#include <string>
#include <vector>

template <typename... Middlewares>
crow::response infoHandler(crow::App<Middlewares...> &app, int game_id) {
  auto connection = makeScopeGuard(pg_pool->connection(),
                                   [](std::shared_ptr<pqxx::connection> conn) {
                                     pg_pool->freeConnection(conn);
                                   });
  pqxx::work txn(*(connection.get()));

  crow::json::wvalue result;
  try {
    GameInfo game_info = readGame(txn, game_id);
    result["game_info"] = game_info.as_json();
  } catch (const std::out_of_range &e) {

    return crow::response(404, e.what());
  }
  std::vector<GameAction> actions = readActions(txn, game_id);
  result["actions"] = crow::json::wvalue(crow::json::type::List);
  std::vector<crow::json::wvalue> actions_json;
  for (auto action : actions) {
    actions_json.push_back(action.as_json());
  }
  result["actions"] = crow::json::wvalue::list(actions_json);
  if (actions.empty()) {
    result["game_state"] = crow::json::load("{}");
  } else {
    try {
      GameState game_state = readState(txn, actions.back().action_id);
      result["game_state"] = game_state.as_json();
    } catch (const std::out_of_range &e) {
      result["game_state"] = crow::json::load("{}");
    }
  }
  return crow::response(result);
}