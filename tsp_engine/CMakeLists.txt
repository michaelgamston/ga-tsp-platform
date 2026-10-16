cmake_minimum_required(VERSION 3.22)
project(ga-tsp-platform LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON) 

add_executable(tsp_engine tsp_engine.cpp)
target_compile_options(tsp_engine PRIVATE -Wall -Wextra)