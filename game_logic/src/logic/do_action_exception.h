#pragma once

#include <exception>
#include <string>

class DoActionException : public std::exception {
public:
  DoActionException(const std::string &message) : msg_(message) {}

  virtual const char *what() const noexcept override { return msg_.c_str(); }

private:
  std::string msg_;
};
