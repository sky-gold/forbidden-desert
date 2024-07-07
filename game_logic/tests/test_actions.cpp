#include "../src/logic/actions/dig.h"
#include "../src/logic/actions/end_of_turn.h"
#include "../src/logic/actions/flip_over.h"
#include "../src/logic/actions/join.h"
#include "../src/logic/actions/leave.h"
#include "../src/logic/actions/move.h"
#include "../src/schemas/game_info.h"
#include "../src/schemas/game_state.h"

#include <crow/json.h>
#include <gtest/gtest.h>
#include <vector>

GameInfo simpleGameInfo() {
  GameInfo game_info;
  game_info.created_at = "2024-07-06 13:01:49.219276";
  game_info.settings = crow::json::load("{\"difficulty\":\"easy\"}");
  game_info.status = "LOBBY";
  game_info.players = {1};
  game_info.id = 1;
  return game_info;
}

GameState startGameState(const GameInfo &game_info) {
  GameState game_state;
  std::vector<std::string> types = {"red",    "blue",  "green",
                                    "yellow", "black", "white"};
  for (int player_id : game_info.players) {
    GameState::PlayerInfo player_info;
    player_info.pos = 0;
    player_info.type = types.back();
    types.pop_back();
    player_info.user_id = player_id;
    player_info.water_amount = 5;
    game_state.players_info.push_back(player_info);
  }
  game_state.player_turn = game_state.players_info[0].user_id;
  game_state.actions_left = 4;
  game_state.board = generateStartBoard();
  return game_state;
}

TEST(JoinTest, JoinTestOk) {
  GameInfo game_info = simpleGameInfo();
  crow::json::rvalue action = crow::json::load("{\"type\": \"join\"}");
  GameInfo new_game_info = joinAction(2, 1, action, game_info);
  ASSERT_EQ(new_game_info.players, std::vector<int>({1, 2}));
}

TEST(LeaveTest, LeaveTestOk) {
  GameInfo game_info = simpleGameInfo();
  game_info.players = std::vector<int>({1, 2});
  crow::json::rvalue action = crow::json::load("{\"type\": \"join\"}");
  GameInfo new_game_info = leaveAction(2, 1, action, game_info);
  ASSERT_EQ(new_game_info.players, std::vector<int>({1}));
}

TEST(MoveTest, MoveTestOk) {
  GameInfo game_info = simpleGameInfo();
  game_info.status = "IN_PROGRESS";
  crow::json::rvalue action =
      crow::json::load("{\"type\": \"move\", \"to\": 1}");
  GameState game_state = startGameState(game_info);
  GameState new_state = moveAction(1, 1, action, game_info, game_state);
  ASSERT_EQ(new_state.players_info[0].pos, 1);
  ASSERT_EQ(new_state.actions_left, 3);
}

TEST(DigTest, DigTestOk) {
  GameInfo game_info = simpleGameInfo();
  game_info.status = "IN_PROGRESS";
  crow::json::rvalue action =
      crow::json::load("{\"type\": \"dig\", \"to\": 1}");
  GameState game_state = startGameState(game_info);
  game_state.board[1].sand_amount = 1;
  GameState new_state = digAction(1, 1, action, game_info, game_state);
  ASSERT_EQ(new_state.board[1].sand_amount, 0);
  ASSERT_EQ(new_state.actions_left, 3);
}

TEST(FlipOverTest, FlipOverTestOk) {
  GameInfo game_info = simpleGameInfo();
  game_info.status = "IN_PROGRESS";
  crow::json::rvalue action = crow::json::load("{\"type\": \"flip_over\"}");
  GameState game_state = startGameState(game_info);
  GameState new_state = flipOverAction(1, 1, action, game_info, game_state);
  ASSERT_TRUE(new_state.board[0].is_up);
  ASSERT_EQ(new_state.actions_left, 3);
}

TEST(EndOfTurnTest, FlipOverTestOk) {
  GameInfo game_info = simpleGameInfo();
  game_info.status = "IN_PROGRESS";
  game_info.players = std::vector<int>({1, 2});
  crow::json::rvalue action = crow::json::load("{\"type\": \"end_of_turn\"}");
  GameState game_state = startGameState(game_info);
  game_state.player_turn = 1;
  game_state.actions_left = 2;
  GameState new_state = endOfTurnAction(1, 1, action, game_info, game_state);
  ASSERT_EQ(new_state.player_turn, 2);
  ASSERT_EQ(new_state.actions_left, 4);
}
