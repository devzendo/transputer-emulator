include_directories(../Shared)
include_directories(../target/classes)
link_libraries(parachutedev)
link_libraries(parachuteversion)

if(PICO)
  link_libraries(pico_stdlib)
endif(PICO)

add_executable(temulate main.cpp memory.cpp cpu.cpp disasm.cpp symbol.cpp opcodes.h)

if(PICO)
  pico_add_extra_outputs(temulate)
endif(PICO)
