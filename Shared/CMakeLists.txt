if(WIN32)
    set(platform_sources namedpipelink.cpp namedpipelink.h)
endif(WIN32)
if(UNIX AND NOT(PICO))
    set(platform_sources fifolink.cpp fifolink.h)
endif(UNIX AND NOT(PICO))
if(PICO)
    # Links can throw, perhaps review that.
    set(PICO_CXX_ENABLE_EXCEPTIONS 1)
#    set(platform_sources )
endif(PICO)
if(EMBEDDED)
    set(non_embedded_sources ) # nothing
else()
    # the library code only needed on desktop (non embedded) builds..
    set(non_embedded_sources stublink.cpp stublink.h tvslink.cpp tvslink.h filesystem.cpp filesystem.h)
endif(EMBEDDED)

message(STATUS "platform_sources: ${platform_sources}")
message(STATUS "non_embedded_sources: ${non_embedded_sources}")

# Note: parachutedev provides the API and part of the implementation of the logging subsystem, but you need to add
# either parachutedesktop, parachutepicostdio or parachutepicousbcdc to get the rest of the implementation.
add_library(parachutedev STATIC log.h logbase.cpp logbase.h misc.h misc.cpp hexdump.h hexdump.cpp link.h link.cpp
        "${platform_sources}"
        linkfactory.h linkfactory.cpp constants.h emuquery.h memloc.h platformdetection.h types.h
        nulllink.cpp nulllink.h
        ringbuffer.cpp ringbuffer.h
        asynclink.h
        gpioasynclink.h gpioasynclink.cpp
        "${non_embedded_sources}"
)

if(NOT(EMBEDDED))
    add_library(parachutedesktop STATIC logdesktop.cpp)
endif() # NOT EMBEDDED

if(PICO)
    add_library(parachutepicostdio STATIC logpicostdio.cpp sync.h)
    target_link_libraries(parachutepicostdio pico_sync pico_stdio)

    add_library(parachutepicousbcdc STATIC logpicousbcdc.cpp sync.h tusb_config.h cdc_app.c usb_descriptors.c picousbseriallink.cpp picousbseriallink.h)
    target_link_libraries(parachutepicousbcdc pico_sync pico_stdlib pico_unique_id tinyusb_device tinyusb_board)
    # pico_stdlib needed by tinyusb
    # Use the submodule of tinyusb in the Raspberry Pi Pico SDK
    #set(PICO_TINYUSB_PATH ${PICO_SDK_DIR}/lib/tinyusb/src)
    # Make sure TinyUSB can find tusb_config.h
    target_include_directories(parachutepicousbcdc PUBLIC ${CMAKE_CURRENT_LIST_DIR}/)
    #target_include_directories(picolinkadapter PUBLIC ${PICO_TINYUSB_PATH})

endif(PICO)

# gpioasynclink is intended for embedded, but am developing it on desktop for now, as it's testable there,
# apart from the actual GPIOTxRxPin.
# if gpioasynclink is built for pico, it pulls in some pico SDK:
if(PICO)
    target_link_libraries(parachutedev pico_sync pico_stdlib ) # what specifically - pico_stdlib is too wide
endif(PICO)

if(NOT(EMBEDDED))
  add_library(testfixtures STATIC tempfilesfixture.h tempfilesfixture.cpp exceptionfixture.h)
  target_link_libraries(testfixtures parachutedev parachutedesktop)

  add_executable(testtypes testtypes.cpp)
  target_link_libraries(testtypes gtest gmock_main parachutedev parachutedesktop)
  add_test(NAME testtypes COMMAND testtypes)

  add_executable(testlink testlink.cpp)
  target_link_libraries(testlink parachutedev parachutedesktop gtest gmock_main)
  add_test(NAME testlink COMMAND testlink)

  add_executable(testasynclink testasynclink.cpp)
  target_link_libraries(testasynclink parachutedev parachutedesktop gtest gmock_main)
  add_test(NAME testasynclink COMMAND testasynclink)

  add_executable(testfilesystem testfilesystem.cpp)
  target_link_libraries(testfilesystem testfixtures parachutedev parachutedesktop gtest gmock_main gsl)
  add_test(NAME testfilesystem COMMAND testfilesystem)

  add_executable(testmisc testmisc.cpp)
  target_link_libraries(testmisc parachutedev parachutedesktop gtest gmock_main)
  add_test(NAME testmisc COMMAND testmisc)

  add_executable(testcharacterisation testcharacterisation.cpp)
  target_link_libraries(testcharacterisation testfixtures parachutedev parachutedesktop gtest gmock_main)
  add_test(NAME testcharacterisation COMMAND testcharacterisation)
  
  add_executable(testringbuffer testringbuffer.cpp)
  target_link_libraries(testringbuffer parachutedev parachutedesktop gtest gmock_main)
  add_test(NAME testringbuffer COMMAND testringbuffer)

endif() # NOT EMBEDDED

