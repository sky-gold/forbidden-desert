cmake_minimum_required(VERSION 3.22.1)
project(game_logic)

find_package(Crow REQUIRED)
find_package(libpqxx REQUIRED)
find_package(OpenSSL REQUIRED)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} Crow::Crow libpqxx::pqxx openssl::openssl)