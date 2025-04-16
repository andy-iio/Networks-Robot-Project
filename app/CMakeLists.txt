cmake_minimum_required(VERSION 3.10)
project(Robot)

set(CMAKE_CXX_STANDARD 17)


set(CROW_PATH ${CMAKE_SOURCE_DIR}/Crow)
include_directories(${CROW_PATH}/include)

add_executable(Robot
    src/main.cpp
    src/Pktdef.cpp
    src/Socket.cpp
)
