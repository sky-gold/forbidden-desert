#include "../src/helpers/env_vars.h"
#include "../src/middlewares/auth.h"
#include <crow/ci_map.h>
#include <crow/http_request.h>
#include <crow/http_response.h>
#include <crow/json.h>
#include <crow/logging.h>
#include <gtest/gtest.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <string>
#include <string_view>

namespace TestingHelpers {

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

std::string create_auth_token(std::string bot_token) {
  std::time_t cur_time = std::time(nullptr);
  std::string data_check_string =
      "auth_date=" + std::to_string(cur_time) +
      "\nfirst_name=name\nid=123\nlast_name=surname\nusername=testuser";
  std::string secret_key = sha256(bot_token);
  std::string_view key_view{secret_key};
  std::string_view msg_view{data_check_string};
  std::string check_hash = hmac_sha256(key_view, msg_view);
  return "{\"id\": 123, \"first_name\": \"name\", \"last_name\": \"surname\", "
         "\"username\": \"testuser\", \"auth_date\": " +
         std::to_string(cur_time) + ", \"hash\": \"" + check_hash + "\"}";
}
} // namespace TestingHelpers

TEST(AuthTesting, test_ok) {
  EnvVars::BOT_TOKEN = "TEST123";
  std::string token = TestingHelpers::create_auth_token(EnvVars::BOT_TOKEN);
  auto auth = AuthGuard();
  crow::request req;
  req.add_header("Authorization", token);
  crow::response res;
  AuthGuard::context ctx;
  auth.before_handle(req, res, ctx);
  ASSERT_NE(res.code, 401);
  ASSERT_EQ(ctx.id, 123);
}

TEST(AuthTesting, test_401) {
  EnvVars::BOT_TOKEN = "TEST123";
  std::string token = TestingHelpers::create_auth_token("WRONG");
  auto auth = AuthGuard();
  crow::request req;
  req.add_header("Authorization", token);
  crow::response res;
  AuthGuard::context ctx;
  auth.before_handle(req, res, ctx);
  ASSERT_EQ(res.code, 401);
}
