add_library(parachutepicousbcdc STATIC logpicousbcdc.cpp tusb_config.h cdc_app.c usb_descriptors.c picousbseriallink.cpp picousbseriallink.h)

target_link_libraries(parachutepicousbcdc parachutedev pico_sync pico_stdlib pico_unique_id tinyusb_device tinyusb_board)
# Use the submodule of tinyusb in the Raspberry Pi Pico SDK
#set(PICO_TINYUSB_PATH ${PICO_SDK_DIR}/lib/tinyusb/src)
# Make sure TinyUSB can find tusb_config.h
target_include_directories(parachutepicousbcdc PUBLIC ${CMAKE_CURRENT_LIST_DIR}/)
#target_include_directories(picolinkadapter PUBLIC ${PICO_TINYUSB_PATH})
pico_enable_stdio_usb(parachutepicousbcdc 0) # Will implement log over USB CDC directly
pico_enable_stdio_uart(parachutepicousbcdc 0)
