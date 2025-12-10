cmake_minimum_required(VERSION 3.10)
project(websocket)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(server server/server.c)

add_executable(client client/client.cpp)
