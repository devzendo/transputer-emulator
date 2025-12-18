include_directories(../Shared)

# Some test harnesses and a link adapter for the pico
#add_executable(picolinkharness picolinkharness.cpp getline.h getline.cpp)
#target_link_libraries(picolinkharness parachutedev parachutepicostdio pico_stdlib)
#pico_enable_stdio_usb(picolinkharness 1)
#pico_enable_stdio_uart(picolinkharness 0)
#pico_add_extra_outputs(picolinkharness)

add_executable(picolinkreflect picolinkreflect.cpp debouncer.h debouncer.cpp)
target_link_libraries(picolinkreflect parachutedev parachutepicostdio pico_rand pico_stdlib)
pico_enable_stdio_usb(picolinkreflect 1)
pico_enable_stdio_uart(picolinkreflect 0)
pico_add_extra_outputs(picolinkreflect)

add_executable(picolinkadapter picolinkadapter.cpp)
# For TinyUSB device support and tinyusb_board for the additional board support library.
# In addition to pico_stdlib required for common PicoSDK functionality, add dependency on tinyusb_device
target_link_libraries(picolinkadapter parachutedev parachutepicousbcdc pico_stdlib)
pico_enable_stdio_usb(picolinkadapter 0) # Will implement log over USB CDC directly
pico_enable_stdio_uart(picolinkadapter 0)
pico_add_extra_outputs(picolinkadapter)

