include_directories(../Shared)
include_directories(../target/classes)
link_libraries(parachutedev)
link_libraries(parachuteversion)
add_executable(temulate main.cpp memory.cpp cpu.cpp disasm.cpp symbol.cpp opcodes.h)

if(PICO)
  target_link_libraries(temulate pico_stdlib)

  # Until the Pico FIFO USB link is rewritten to use tinyusb...
  pico_enable_stdio_usb(temulate 1)
  pico_enable_stdio_uart(temulate 0)

  pico_add_extra_outputs(temulate)
endif(PICO)
