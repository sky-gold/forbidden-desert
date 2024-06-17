#include "env_vars.h"

#include <crow.h>
#include <crow/logging.h>
#include <cstdlib>
#include <string>

namespace EnvVars {
std::string BOT_TOKEN;
std::string DB_HOST;
std::string DB_NAME;
std::string DB_USER;
std::string DB_PASSWORD;

std::string getEnvironmentVariable(const std::string &variableName) {
  const char *value = std::getenv(variableName.c_str());
  if (value == nullptr) {
    CROW_LOG_ERROR << "EnvironmentVariable " << variableName << " not exist";
    return std::string();
  }
  return std::string(value);
}

void init() {
  BOT_TOKEN = getEnvironmentVariable("BOT_TOKEN");
  // BOT_TOKEN = "TEST123";
  DB_HOST = getEnvironmentVariable("DB_HOST");
  DB_NAME = getEnvironmentVariable("DB_NAME");
  DB_USER = getEnvironmentVariable("DB_USER");
  DB_PASSWORD = getEnvironmentVariable("DB_PASSWORD");
}
} // namespace EnvVars