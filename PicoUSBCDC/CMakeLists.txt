add_library(parachutepicousbcdc STATIC cdc_app.c cdc_app.h usb_descriptors.c usb_descriptors.h
        logpicousbcdc.cpp picousbseriallink.cpp picousbseriallink.h tusb_config.h)

target_link_libraries(parachutepicousbcdc parachutedev pico_unique_id tinyusb_device tinyusb_board)
# Use the submodule of tinyusb in the Raspberry Pi Pico SDK
#set(PICO_TINYUSB_PATH ${PICO_SDK_DIR}/lib/tinyusb/src)
# Make sure TinyUSB can find tusb_config.h
target_include_directories(parachutepicousbcdc PUBLIC ${CMAKE_CURRENT_LIST_DIR}/)
#target_include_directories(picolinkadapter PUBLIC ${PICO_TINYUSB_PATH})
pico_enable_stdio_usb(parachutepicousbcdc 0) # Will implement log over USB CDC directly
pico_enable_stdio_uart(parachutepicousbcdc 0)
