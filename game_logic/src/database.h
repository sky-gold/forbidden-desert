#include <pqxx/pqxx>

class Database {
public:
  Database(const std::string &connectionString)
      : m_connectionString(connectionString) {}

  pqxx::result executeQuery(const std::string &query) {
    pqxx::connection conn(m_connectionString);
    pqxx::work txn(conn);
    pqxx::result res = txn.exec(query);
    txn.commit();
    return res;
  }

private:
  std::string m_connectionString;
};