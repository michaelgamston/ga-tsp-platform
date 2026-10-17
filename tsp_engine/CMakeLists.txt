cmake_minimum_required(VERSION 3.22)
project(ga-tsp-platform LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Core library: all engine code except main() ----
add_library(tsp_core
    src/map_io.cpp
)
target_include_directories(tsp_core PUBLIC include)
target_compile_options(tsp_core PRIVATE -Wall -Wextra)

# ---- Engine executable: just main(), linked to the library ----
add_executable(tsp_engine src/tsp_engine.cpp)
target_link_libraries(tsp_engine PRIVATE tsp_core)
target_compile_options(tsp_engine PRIVATE -Wall -Wextra)

# ---- GoogleTest: downloaded automatically at configure time ----
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

# ---- Tests ----
enable_testing()

add_executable(engine_tests tests/test_map_io.cpp)
target_link_libraries(engine_tests PRIVATE tsp_core GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(engine_tests)
