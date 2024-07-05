#pragma once

#include <crow/json.h>

class ActionValidationException : public std::exception {
public:
  ActionValidationException(const std::string &message) : msg_(message) {}

  virtual const char *what() const noexcept override { return msg_.c_str(); }

private:
  std::string msg_;
};

void actionValidate(const crow::json::rvalue &action);