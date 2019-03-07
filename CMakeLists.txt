cmake_minimum_required(VERSION 3.10.2)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# VERSION is filtered into target/classes/version.cpp by using the maven resources plugin.
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -Wall -O2 -g -DDEBUG -DVERSION=\"${VERSION}\"")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -g -DDEBUG -DVERSION=\"${VERSION}\"")
project(transputer-emulator)

add_subdirectory(target/classes)
add_subdirectory(Shared)
add_subdirectory(NodeServer)
add_subdirectory(Emulator)
