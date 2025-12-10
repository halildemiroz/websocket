cmake_minimum_required(VERSION 3.10)
project(websocket)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Fix for Homebrew on M1/M2/M3 Macs
set(BOOST_ROOT "/opt/homebrew")
set(Boost_INCLUDE_DIR "/opt/homebrew/include")
set(Boost_LIBRARY_DIR "/opt/homebrew/lib")

# Find Boost (Headers only is fine for Beast usually)
find_package(Boost REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})

add_executable(server server/server.c)

add_executable(client client/client.cpp)


# Beast is header-only, but we might need system linkage
if(Boost_LIBRARIES)
    target_link_libraries(server ${Boost_LIBRARIES})
    target_link_libraries(client ${Boost_LIBRARIES})
endif()
