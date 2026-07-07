include_directories(../Shared)
include_directories(../IServer)
include_directories(../IServer/server)
include_directories(../target/classes)
link_libraries(parachuteversion)

add_executable(emuserver emuserver.cpp)
target_link_libraries(emuserver parachutedesktop parachuteemulator parachuteiserver)
