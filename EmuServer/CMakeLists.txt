include_directories(../Shared)
include_directories(../Emulator)
include_directories(../IServer)
include_directories(../IServer/server)
include_directories(../target/classes)
link_libraries(parachuteversion)

add_executable(emuserver emuserver.cpp)
target_link_libraries(emuserver parachutedesktop parachuteemulator parachuteiserver)
# This inclusion of google tests' include path to the emuserver executable is so that test
# code can be a friend of the link code and test internals correctly without needing to expose them publically.
target_include_directories(emuserver PRIVATE ${gtest_SOURCE_DIR}/include)
