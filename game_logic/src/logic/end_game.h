#pragma once

#include <pqxx/pqxx>

void TryToEnd(pqxx::work &txn, int game_id);