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
#include <string_view>
#include <vector>

std::string sha256(const std::string &input) {
  unsigned char hash[SHA256_DIGEST_LENGTH];
  SHA256_CTX sha256;
  SHA256_Init(&sha256);
  SHA256_Update(&sha256, input.c_str(), input.size());
  SHA256_Final(hash, &sha256);

  std::string res;
  for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
    res.push_back(hash[i]);
  }
  return res;
}

std::string hmac_sha256(std::string_view decodedKey, std::string_view msg) {
  std::array<unsigned char, EVP_MAX_MD_SIZE> hash;
  unsigned int hashLen;
  HMAC(EVP_sha256(), decodedKey.data(), static_cast<int>(decodedKey.size()),
       reinterpret_cast<unsigned char const *>(msg.data()),
       static_cast<int>(msg.size()), hash.data(), &hashLen);
  std::stringstream out;
  for (unsigned int i = 0; i < hashLen; i++) {
    out << std::setfill('0') << std::setw(2) << std::right << std::hex
        << (int)hash.data()[i];
  }
  return out.str();
}

void AuthGuard::before_handle(crow::request &req, crow::response &res,
                              AuthGuard::context &ctx) {
  try {
    auto authHeaderIt = req.headers.find("Authorization");
    if (authHeaderIt == req.headers.end()) {
      CROW_LOG_INFO << "No Authorization Header";
      res.code = 401;
      res.end();
      return;
    }
    crow::json::rvalue data;
    try {
      data = crow::json::load(authHeaderIt->second);
    } catch (const std::exception &e) {
      CROW_LOG_INFO << "Bad Authorization Header: " << e.what();
      res.code = 401;
      res.end();
      return;
    }

    if (!data.has("hash") || !data.has("id") || !data.has("auth_date")) {
      CROW_LOG_INFO << "Authorization JSON does not have required fields";
      res.code = 400;
      res.end();
      return;
    }
    std::vector<std::string> v;
    for (std::string key : data.keys()) {
      if (key == "hash") {
        continue;
      }
      std::string value = std::string(data[key]);
      v.push_back(key + "=" + value);
    }
    sort(v.begin(), v.end());
    std::string data_check_string;
    for (auto s : v) {
      if (!data_check_string.empty()) {
        data_check_string += "\n";
      }
      data_check_string += s;
    }
    std::string secret_key = sha256(EnvVars::BOT_TOKEN);
    std::string_view key_view{secret_key};
    std::string_view msg_view{data_check_string};
    std::string check_hash = hmac_sha256(key_view, msg_view);
    if (check_hash != data["hash"].s()) {
      CROW_LOG_INFO << "Authorization JSON bad hash";
      res.code = 401;
      res.end();
      return;
    }

    std::time_t cur_time = std::time(nullptr);

    // CROW_LOG_INFO << "auth_date=" << data["auth_date"].i();

    if (std::abs(data["auth_date"].i() - cur_time) > 365 * (24 * 60 * 60)) {
      CROW_LOG_INFO << "Authorization JSON expired";
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