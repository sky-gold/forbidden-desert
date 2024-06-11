#include "auth.h"

#include "../helpers/env_vars.h"

#include <crow/ci_map.h>
#include <crow/http_request.h>
#include <crow/http_response.h>
#include <crow/json.h>
#include <crow/logging.h>
#include <iomanip>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <string>
#include <vector>

std::string calculate_hash(const std::string &data, const std::string &key) {
  unsigned char *result;
  unsigned int len = SHA256_DIGEST_LENGTH;
  result = HMAC(EVP_sha256(), key.c_str(), key.size(),
                (unsigned char *)data.c_str(), data.size(), NULL, &len);

  std::stringstream ss;
  for (int i = 0; i < len; i++) {
    ss << std::hex << std::setw(2) << std::setfill('0') << (int)result[i];
  }

  return ss.str();
}

void AuthGuard::before_handle(crow::request &req, crow::response &res,
                              AuthGuard::context &ctx) {
  try {
    auto authHeaderIt = req.headers.find("Authorization");
    if (authHeaderIt == req.headers.end()) {
      res.code = 401;
      res.end();
      CROW_LOG_INFO << "No Authorization Header";
      return;
    }
    crow::json::rvalue data;
    try {
      data = crow::json::load(authHeaderIt->second);
    } catch (const std::exception &e) {
      res.code = 401;
      res.end();
      CROW_LOG_INFO << "Bad Authorization Header: " << e.what();
      return;
    }

    if (!data.has("hash") || !data.has("id") || !data.has("auth_date")) {
      res.code = 400;
      res.end();
      CROW_LOG_INFO << "Authorization JSON does not have required fields";
      return;
    }
    std::vector<std::string> v;
    for (std::string key : data.keys()) {
      std::string value = std::string(data[key]);
      v.push_back(key + "=" + value);
    }
    sort(v.begin(), v.end());
    std::string data_check_string;
    for (auto s : v) {
      if (!v.empty()) {
        data_check_string += "\n";
      }
      data_check_string += s;
    }
    // CROW_LOG_INFO << "data_check_string=" << data_check_string;
    std::string secret_key = calculate_hash(EnvVars::BOT_TOKEN, "");
    std::string check_hash = calculate_hash(data_check_string, secret_key);
    // CROW_LOG_INFO << "secret_key=" << secret_key;
    // CROW_LOG_INFO << "check_hash=" << check_hash;

    std::time_t cur_time = std::time(nullptr);

    // CROW_LOG_INFO << "auth_date=" << data["auth_date"].i();

    if (std::abs(data["auth_date"].i() - cur_time) > 4 * (24 * 60 * 60)) {
      res.code = 401;
      res.end();
      return;
    }
    ctx.id = data["id"].i();
  } catch (const std::exception &e) {
    CROW_LOG_WARNING << "Auth Exception: " << e.what();
    res.code = 500;
    res.end();
  }
}