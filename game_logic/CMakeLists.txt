cmake_minimum_required(VERSION 3.22.1)
project(game_logic)

find_package(Crow REQUIRED)
find_package(libpqxx REQUIRED)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} Crow::Crow libpqxx::pqxx)