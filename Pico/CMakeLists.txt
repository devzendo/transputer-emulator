include_directories(../Shared)
include_directories(../PicoShared)

# Some test harnesses and a link adapter for the pico
add_executable(picolinkharness picolinkharness.cpp getline.h getline.cpp )
target_link_libraries(picolinkharness parachutedev pico_stdlib)
pico_enable_stdio_usb(picolinkharness 1)
pico_enable_stdio_uart(picolinkharness 0)
pico_add_extra_outputs(picolinkharness)

add_executable(picolinkreflect picolinkreflect.cpp debouncer.h debouncer.cpp )
target_link_libraries(picolinkreflect parachutedev pico_rand pico_stdlib)
pico_enable_stdio_usb(picolinkreflect 1)
pico_enable_stdio_uart(picolinkreflect 0)
pico_add_extra_outputs(picolinkreflect)

add_executable(picolinkadapter picolinkadapter.cpp)
# For TinyUSB device support and tinyusb_board for the additional board support library.
# In addition to pico_stdlib required for common PicoSDK functionality, add dependency on tinyusb_device
target_link_libraries(picolinkadapter parachutedev parachutepicousbcdc pico_unique_id tinyusb_device tinyusb_board)
# Use the submodule of tinyusb in the Raspberry Pi Pico SDK
#set(PICO_TINYUSB_PATH ${PICO_SDK_DIR}/lib/tinyusb/src)
# Make sure TinyUSB can find tusb_config.h
target_include_directories(picolinkadapter PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../Shared)
#target_include_directories(picolinkadapter PUBLIC ${PICO_TINYUSB_PATH})
# Will implement log and link over USB CDC directly
pico_add_extra_outputs(picolinkadapter)

