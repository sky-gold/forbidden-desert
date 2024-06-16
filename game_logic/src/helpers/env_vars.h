#pragma once

#include <string>

namespace EnvVars {

extern std::string BOT_TOKEN;
extern std::string DB_HOST;
extern std::string DB_NAME;
extern std::string DB_USER;
extern std::string DB_PASSWORD;

void init();

} // namespace EnvVars
