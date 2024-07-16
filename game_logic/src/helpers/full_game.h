#pragma once

#include <crow/json.h>
#include <pqxx/pqxx>

crow::json::wvalue readFullGameInfo(pqxx::work &txn, int game_id,
                                    bool return_actions = true);