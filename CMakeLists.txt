cmake_minimum_required(VERSION 3.25)
project(kernSched LANGUAGES CXX)

set(CMAKE_CXX_COMPILER "/home/kweronsx/dpcpp/build/bin/clang++")
set(CMAKE_C_COMPILER "/home/kweronsx/dpcpp/build/bin/clang")

set(CMAKE_CXX_STANDARD_REQUIRED 1)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(src)
