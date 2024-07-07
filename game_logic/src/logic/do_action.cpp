#include "do_action.h"
#include "../schemas/game_action.h"
#include "../schemas/game_info.h"
#include "../schemas/game_state.h"
#include "actions/dig.h"
#include "actions/end_of_turn.h"
#include "actions/flip_over.h"
#include "actions/join.h"
#include "actions/leave.h"
#include "actions/move.h"
#include "actions/start.h"
#include "do_action_exception.h"

#include <crow/json.h>
#include <crow/logging.h>
#include <pqxx/pqxx>
#include <stdexcept>
#include <string>
#include <vector>

void addActionAndState(pqxx::work &txn, int game_id, int user_id,
                       const crow::json::rvalue &action, GameState &new_state) {
  GameAction game_action;
  game_action.game_id = game_id;
  game_action.type = action["type"].s();
  game_action.info = action;
  game_action.user_id = user_id;
  game_action.action_id = addAction(txn, game_action);
  new_state.action_id = game_action.action_id;
  addState(txn, new_state);
}

void doAction(pqxx::work &txn, int user_id, int game_id,
              const crow::json::rvalue &action) {
  CROW_LOG_INFO << "doAction " << action;
  GameInfo game_info;
  try {
    game_info = readGame(txn, game_id);
  } catch (const std::out_of_range &e) {
    throw DoActionException(e.what());
  }
  if (game_info.status == "FINISHED") {
    throw DoActionException("game is finished");
  }
  if (game_info.status == "CANCELED") {
    throw DoActionException("game is canceled");
  }
  std::string action_type = action["type"].s();
  if (action_type == "join") {
    updateGameInfo(txn, joinAction(user_id, game_id, action, game_info));
    return;
  }
  if (action_type == "leave") {
    updateGameInfo(txn, leaveAction(user_id, game_id, action, game_info));
    return;
  }
  if (action_type == "start") {
    std::pair<GameInfo, GameState> res =
        startAction(user_id, game_id, action, game_info);
    updateGameInfo(txn, res.first);
    addActionAndState(txn, game_id, user_id, action, res.second);
    return;
  }
  std::vector<GameAction> actions = readActions(txn, game_id);
  GameState game_state = readState(txn, actions.back().action_id);
  if (action_type == "end_of_turn") {
    GameState new_state =
        endOfTurnAction(user_id, game_id, action, game_info, game_state);
    addActionAndState(txn, game_id, user_id, action, new_state);
    return;
  }
  if (action_type == "move") {
    GameState new_state =
        moveAction(user_id, game_id, action, game_info, game_state);
    addActionAndState(txn, game_id, user_id, action, new_state);
    return;
  }
  if (action_type == "dig") {
    GameState new_state =
        digAction(user_id, game_id, action, game_info, game_state);
    addActionAndState(txn, game_id, user_id, action, new_state);
    return;
  }
  if (action_type == "flip_over") {
    GameState new_state =
        flipOverAction(user_id, game_id, action, game_info, game_state);
    addActionAndState(txn, game_id, user_id, action, new_state);
    return;
  }
  throw DoActionException("Not implemented action: " + action_type);
}