include_directories(../Shared)

# Some test harnesses and a link adapter for the pico

# TODO The link harness and link reflector both use pico stdio, and the link adapter uses the 2xUSB CDC log/link code.
# The pico stdio code looks right but does not build - my suspicion is that the CDC USB code is interfering with it,
# so knock the two pico stdio users out for now.
#add_executable(picolinkharness picolinkharness.cpp getline.h getline.cpp)
#target_link_libraries(picolinkharness parachutedev parachutepicostdio pico_stdlib pico_stdio)
#pico_enable_stdio_usb(picolinkharness 1)
#pico_enable_stdio_uart(picolinkharness 0)
#pico_add_extra_outputs(picolinkharness)
#
#add_executable(picolinkreflect picolinkreflect.cpp debouncer.h debouncer.cpp)
#target_link_libraries(picolinkreflect parachutedev parachutepicostdio pico_rand pico_stdlib pico_stdio)
#pico_enable_stdio_usb(picolinkreflect 1)
#pico_enable_stdio_uart(picolinkreflect 0)
#pico_add_extra_outputs(picolinkreflect)

add_executable(picolinkadapter picolinkadapter.cpp)
# For TinyUSB device support and tinyusb_board for the additional board support library.
# In addition to pico_stdlib required for common PicoSDK functionality, add dependency on tinyusb_device
target_link_libraries(picolinkadapter parachutedev parachutepicousbcdc)
# Use the submodule of tinyusb in the Raspberry Pi Pico SDK
#set(PICO_TINYUSB_PATH ${PICO_SDK_DIR}/lib/tinyusb/src)
# Make sure TinyUSB can find tusb_config.h
target_include_directories(picolinkadapter PUBLIC ${CMAKE_CURRENT_LIST_DIR}/)
#target_include_directories(picolinkadapter PUBLIC ${PICO_TINYUSB_PATH})
pico_enable_stdio_usb(picolinkadapter 0) # Will implement log over USB CDC directly
pico_enable_stdio_uart(picolinkadapter 0)
pico_add_extra_outputs(picolinkadapter)

