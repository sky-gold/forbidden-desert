#include "action_validate.h"

#include <crow/json.h>
#include <crow/logging.h>
#include <exception>

bool validateJoinAction(const crow::json::rvalue &action) {
  return action.size() == 1;
}

bool validateLeaveAction(const crow::json::rvalue &action) {
  return action.size() == 1;
}

bool actionValidate(const crow::json::rvalue &action) {
  try {
    if (action.t() != crow::json::type::Object || !action.has("type")) {
      return false;
    }
    std::string action_type = action["type"].s();
    if (action_type == "join") {
      return validateJoinAction(action);
    }
    if (action_type == "leave") {
      return validateLeaveAction(action);
    }
    return false;
  } catch (const std::exception &e) {
    CROW_LOG_WARNING << "action=" << action;
    CROW_LOG_WARNING << "action_exception " << e.what();
    return false;
  }
}