cmake_minimum_required(VERSION 4.1)
project(websocket)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(picosec)

add_executable(server server/server.cpp)

