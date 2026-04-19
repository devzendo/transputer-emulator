include_directories(../Shared)
include_directories(../target/classes)
link_libraries(parachutedev)
link_libraries(parachuteversion)
add_executable(temulate main.cpp memory.cpp cpu.cpp disasm.cpp symbol.cpp boot.cpp opcodes.h)

if(PICO)
  target_link_libraries(temulate pico_stdlib parachutepicousbcdc)
  pico_add_extra_outputs(temulate)
else()
  target_link_libraries(temulate parachutedesktop)
endif(PICO)

if(NOT(EMBEDDED))
  # symbol.cpp is coupled to memory.cpp
  add_executable(testboot testboot.cpp boot.cpp memory.cpp symbol.cpp flags.h)
  target_link_libraries(testboot gtest gmock_main parachutedesktop)
  add_test(NAME testboot COMMAND testboot)
endif(NOT(EMBEDDED))
