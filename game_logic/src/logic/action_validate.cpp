#include "action_validate.h"

#include <crow/json.h>
#include <crow/logging.h>
#include <cstddef>
#include <string>

void validateSize(const crow::json::rvalue &action, size_t expected_size) {
  if (action.size() != expected_size) {
    throw ActionValidationException("Invalid action size, expected " +
                                    std::to_string(expected_size) + " get " +
                                    std::to_string(action.size()));
  }
}

void requiredField(const crow::json::rvalue &action, const std::string &name,
                   crow::json::type field_type) {
  if (!action.has(name)) {
    throw ActionValidationException("No required field " + name);
  }
  if (action[name].t() != field_type) {
    throw ActionValidationException(
        "Field \"" + name + "\" has invalid type, expected " +
        crow::json::get_type_str(field_type) + " get " +
        crow::json::get_type_str(action[name].t()));
  }
}

void actionValidate(const crow::json::rvalue &action) {
  if (action.t() != crow::json::type::Object) {
    throw ActionValidationException("Action is not an object");
  }
  requiredField(action, "type", crow::json::type::String);
  std::string action_type = action["type"].s();
  if (action_type == "join") {
    validateSize(action, 1);
    return;
  }
  if (action_type == "leave") {
    validateSize(action, 1);
    return;
  }
  if (action_type == "start") {
    validateSize(action, 1);
    return;
  }
  if (action_type == "end_of_turn") {
    validateSize(action, 1);
    return;
  }
  if (action_type == "move") {
    requiredField(action, "to", crow::json::type::Number);
    validateSize(action, 2);
    return;
  }
  if (action_type == "flip_over") {
    validateSize(action, 1);
    return;
  }
  if (action_type == "dig") {
    requiredField(action, "to", crow::json::type::Number);
    validateSize(action, 2);
    return;
  }
  if (action_type == "take_detail") {
    validateSize(action, 1);
    return;
  }
  if (action_type == "give_water") {
    requiredField(action, "whom", crow::json::type::Number);
    requiredField(action, "amount", crow::json::type::Number);
    validateSize(action, 2);
  }

  throw ActionValidationException("Unknown action type: " + action_type);
}