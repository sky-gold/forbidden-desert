cmake_minimum_required(VERSION 3.22.1)
project(game_logic)

set(CMAKE_MODULE_PATH              "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Crow REQUIRED)
find_package(libpqxx REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(GTest REQUIRED)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} Crow::Crow libpqxx::pqxx openssl::openssl)



# Enable testing
enable_testing()

# Create a list to store test targets
set(TEST_TARGETS "")
set_property(GLOBAL PROPERTY TEST_TARGETS "")

function(add_gtest _target)
    set(sources_list "")
    foreach(source ${ARGN})
        list(APPEND sources_list ${source})
    endforeach()
    add_executable(${_target} ${sources_list})
    add_test(NAME ${_target} COMMAND ${_target})
    target_link_libraries(${_target} GTest::gtest GTest::gtest_main Crow::Crow libpqxx::pqxx openssl::openssl)
    
    # Append the new test target to the list
    set_property(GLOBAL APPEND PROPERTY TEST_TARGETS ${_target})
endfunction()

add_subdirectory(tests)
get_property(TEST_TARGETS GLOBAL PROPERTY TEST_TARGETS)

# Use the list of test targets in the run_tests custom target
add_custom_target(run_tests COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure)
add_dependencies(run_tests ${TEST_TARGETS})
