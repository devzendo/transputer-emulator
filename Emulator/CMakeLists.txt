include_directories(../Shared)
include_directories(../target/classes)
link_libraries(parachutedev)
link_libraries(parachuteversion)

add_library(parachuteemulator STATIC memory.cpp cpu.cpp disasm.cpp symbol.cpp boot.cpp opcodes.h)
# This inclusion of google tests' include path to the parachuteemulator library, temulate executable is so that test
# code can be a friend of the link code and test internals correctly without needing to expose them publically.
target_include_directories(parachuteemulator PRIVATE ${gtest_SOURCE_DIR}/include)

add_executable(temulate temulate.cpp)
target_include_directories(temulate PRIVATE ${gtest_SOURCE_DIR}/include)

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

  add_executable(testcpu testcpu.cpp cpu.cpp boot.cpp memory.cpp symbol.cpp flags.h disasm.cpp disasm.h)
  target_link_libraries(testcpu gtest gmock_main parachutedesktop)
  add_test(NAME testcpu COMMAND testcpu)
endif(NOT(EMBEDDED))
