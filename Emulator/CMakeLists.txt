include_directories(../Shared)
link_libraries(parachutedev)

add_executable(t800emul main.cpp memory.cpp cpu.cpp disasm.cpp opcodes.h)
