#include "helpers/env_vars.h"
#include "middlewares/auth.h"

#include <crow.h>
#include <crow/logging.h>
#include <crow/middleware_context.h>

int main() {
  EnvVars::init();

  crow::App<AuthGuard> app;

  CROW_ROUTE(app, "/")([]() { return "Hello, World!"; });

  app.port(18080).multithreaded().run();
  return 0;
}