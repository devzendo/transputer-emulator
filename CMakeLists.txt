cmake_minimum_required(VERSION 2.6.4)
# set(CMAKE_CXX_STANDARD 11)

file(STRINGS "VERSION.txt" VERSION)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -Wall -O2 -g -DDEBUG -DVERSION=${VERSION}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -g -DDEBUG -DVERSION=${VERSION}")
project(transputer-emulator)

add_subdirectory(Shared)
add_subdirectory(NodeServer)
add_subdirectory(Emulator)
