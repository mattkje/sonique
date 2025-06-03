cmake_minimum_required(VERSION 3.31)
project(untitled)

set(CMAKE_CXX_STANDARD 20)

find_package(raylib 5.0 REQUIRED) # Make sure Raylib is installed

add_executable(untitled main.cpp)
target_link_libraries(untitled PRIVATE raylib)