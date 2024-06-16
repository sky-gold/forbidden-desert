#include "handlers/get_create.h"
#include "helpers/connection_pool.h"
#include "helpers/env_vars.h"
#include "middlewares/auth.h"

#include <crow.h>
#include <crow/app.h>
#include <crow/http_request.h>
#include <crow/logging.h>
#include <crow/middleware_context.h>
#include <string>

int main() {
  EnvVars::init();
  db_init();

  crow::App<AuthGuard> app;

  CROW_ROUTE(app, "/create")
  ([&app](const crow::request &req) { return createHandler(app, req); });

  app.port(18080).multithreaded().run();
  return 0;
}