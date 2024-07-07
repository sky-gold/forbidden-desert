file(GLOB_RECURSE logic_cpp "${CMAKE_CURRENT_SOURCE_DIR}/../src/logic/*.cpp")
file(GLOB_RECURSE schemas_cpp "${CMAKE_CURRENT_SOURCE_DIR}/../src/schemas/*.cpp")
add_gtest(test_middlewares test_middlewares.cpp ../src/helpers/env_vars.cpp ../src/middlewares/auth.cpp)
add_gtest(test_actions test_actions.cpp ${schemas_cpp} ${logic_cpp})
