include_directories(../Shared)
include_directories(../target/classes)
link_libraries(parachutedev)
link_libraries(parachuteversion)

add_library(parachuteemulator STATIC memory.cpp cpu.cpp disasm.cpp symbol.cpp boot.cpp opcodes.h)

add_executable(temulate main.cpp)

if(PICO)
  target_link_libraries(temulate pico_stdlib parachutepicousbcdc parachuteemulator)
  pico_add_extra_outputs(temulate)
else()
  target_link_libraries(temulate parachutedesktop parachuteemulator)
endif(PICO)

if(NOT(EMBEDDED))
  # symbol.cpp is coupled to memory.cpp
  add_executable(testboot testboot.cpp boot.cpp memory.cpp symbol.cpp flags.h)
  target_link_libraries(testboot gtest gmock_main parachutedesktop)
  add_test(NAME testboot COMMAND testboot)
endif(NOT(EMBEDDED))
