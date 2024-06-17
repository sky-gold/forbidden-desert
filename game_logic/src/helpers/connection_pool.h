#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include <pqxx/pqxx>
#include <queue>

class ConnectionPool {
public:
  ConnectionPool(std::string connection_string);
  std::shared_ptr<pqxx::connection> connection();
  void freeConnection(std::shared_ptr<pqxx::connection>);

private:
  void createPool();

  const std::string connection_string;
  std::mutex m_mutex;
  std::condition_variable m_condition;
  std::queue<std::shared_ptr<pqxx::connection>> m_pool;

  const int POOL_SIZE = 10;
};

extern std::shared_ptr<ConnectionPool> pg_pool;

void db_init();