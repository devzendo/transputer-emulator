include_directories(../Shared)
include_directories(../target/classes)
link_libraries(parachutedev)
link_libraries(parachuteversion)

add_executable(bin2boot bin2boot.cpp)
target_link_libraries(bin2boot parachutedesktop)
