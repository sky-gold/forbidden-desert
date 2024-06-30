#pragma once

#include <crow/json.h>
#include <pqxx/pqxx>

void doAction(pqxx::work &txn, int user_id, int game_id,
              const crow::json::rvalue &action);