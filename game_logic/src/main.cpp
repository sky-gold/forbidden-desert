#include <crow.h>
#include <crow/middleware_context.h>

int main() {
  crow::SimpleApp app;

  CROW_ROUTE(app, "/")([]() { return "Hello, World!"; });

  app.port(18080).multithreaded().run();
  return 0;
}