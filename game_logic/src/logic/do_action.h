#pragma once

#include <crow/json.h>
#include <pqxx/pqxx>

class DoActionException : public std::exception {
public:
  DoActionException(const std::string &message) : msg_(message) {}

  virtual const char *what() const noexcept override { return msg_.c_str(); }

private:
  std::string msg_;
};

void doAction(pqxx::work &txn, int user_id, int game_id,
              const crow::json::rvalue &action);