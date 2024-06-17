#include "env_vars.h"

#include "connection_pool.h"
#include <crow/logging.h>
#include <memory>
#include <string>

ConnectionPool::ConnectionPool(std::string connection_string)
    : connection_string(connection_string) {
  createPool();
}

void ConnectionPool::createPool() {
  std::lock_guard<std::mutex> locker_(m_mutex);

  for (auto i = 0; i < POOL_SIZE; i++) {
    m_pool.emplace(
        std::make_shared<pqxx::connection>(connection_string.c_str()));
  }
}

std::shared_ptr<pqxx::connection> ConnectionPool::connection() {
  std::unique_lock<std::mutex> lock_(m_mutex);

  // if pool is empty, then wait until it notifies back
  while (m_pool.empty()) {
    m_condition.wait(lock_);
  }

  // get new connection in queue
  auto conn_ = m_pool.front();
  // immediately pop as we will use it now
  m_pool.pop();

  return conn_;
}

void ConnectionPool::freeConnection(std::shared_ptr<pqxx::connection> conn_) {
  std::unique_lock<std::mutex> lock_(m_mutex);

  // push a new connection into a pool
  m_pool.push(conn_);

  // unlock mutex
  lock_.unlock();

  // notify one of thread that is waiting
  m_condition.notify_one();
}

std::shared_ptr<ConnectionPool> pg_pool;

void db_init() {
  try {
    pg_pool = std::make_shared<ConnectionPool>(
        "dbname=" + EnvVars::DB_NAME + " user=" + EnvVars::DB_USER +
        " password=" + EnvVars::DB_PASSWORD + " host=" + EnvVars::DB_HOST +
        " port=5432");
  } catch (const std::exception &e) {
    CROW_LOG_ERROR << "Bad DB connection: " << e.what();
  }
}