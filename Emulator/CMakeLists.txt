include_directories(../Shared)
link_libraries(parachutedev)

add_executable(temulate main.cpp memory.cpp cpu.cpp disasm.cpp opcodes.h)
