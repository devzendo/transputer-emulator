include_directories(../../Shared)
include_directories(../../target/classes)
include_directories(..)
link_libraries(parachutedev)
link_libraries(parachuteversion)

add_executable(nodeserver nodeserver.cpp)