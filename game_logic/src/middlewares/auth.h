#pragma once

#include <crow/ci_map.h>
#include <crow/http_request.h>
#include <crow/http_response.h>
#include <crow/json.h>
#include <crow/logging.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>

struct AuthGuard {
  struct context {
    int id;
  };

  void before_handle(crow::request &req, crow::response &res, context &ctx);

  void after_handle(crow::request &req, crow::response &res, context &ctx) {}
};