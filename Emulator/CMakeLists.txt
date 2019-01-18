include_directories(../Shared)
include_directories(../target/classes)
link_libraries(parachutedev)
link_libraries(parachuteversion)

add_executable(temulate main.cpp memory.cpp cpu.cpp disasm.cpp opcodes.h)
