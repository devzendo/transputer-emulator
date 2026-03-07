add_library(parachutepicostdio STATIC logpicostdio.cpp)
target_link_libraries(parachutepicostdio parachutedev pico_sync pico_stdio)

# Use the submodule of tinyusb in the Raspberry Pi Pico SDK
#set(PICO_TINYUSB_PATH ${PICO_SDK_DIR}/lib/tinyusb/src)
# Make sure TinyUSB can find tusb_config.h
#target_include_directories(parachutepicousbcdc PUBLIC ${CMAKE_CURRENT_LIST_DIR}/)
#target_include_directories(picolinkadapter PUBLIC ${PICO_TINYUSB_PATH})
pico_enable_stdio_usb(parachutepicostdio 1)
pico_enable_stdio_uart(parachutepicostdio 0)
