include_directories(../Shared)

add_library(parachutepicousbcdc STATIC ../Shared/sync.h cdc_app.c cdc_app.h usb_descriptors.c usb_descriptors.h picousbseriallink.cpp picousbseriallink.h tusb_config.h)
target_link_libraries(parachutepicousbcdc pico_sync pico_stdlib pico_unique_id tinyusb_device tinyusb_board)
# Make sure TinyUSB can find tusb_config.h
target_include_directories(parachutepicousbcdc PUBLIC ${CMAKE_CURRENT_LIST_DIR}/)
