if(WIN32)
    set(platform_sources namedpipelink.cpp namedpipelink.h)
endif(WIN32)
if(UNIX AND NOT(PICO))
    set(platform_sources fifolink.cpp fifolink.h)
endif(UNIX AND NOT(PICO))
if(PICO)
    # Links can throw, perhaps review that.
    set(PICO_CXX_ENABLE_EXCEPTIONS 1)
    set(platform_sources picousbseriallink.cpp picousbseriallink.h)
endif(PICO)
if(EMBEDDED)
    set(non_embedded_sources ) # nothing
else()
    # the library code only needed on desktop (non embedded) builds..
    set(non_embedded_sources stublink.cpp stublink.h tvslink.cpp tvslink.h filesystem.cpp filesystem.h)
endif(EMBEDDED)

message(STATUS "platform_sources: ${platform_sources}")
message(STATUS "non_embedded_sources: ${non_embedded_sources}")

add_library(parachutedev STATIC log.h log.cpp misc.h misc.cpp hexdump.h hexdump.cpp link.h link.cpp
        "${platform_sources}"
        linkfactory.h linkfactory.cpp constants.h emuquery.h memloc.h platformdetection.h types.h
        nulllink.cpp nulllink.h
        ringbuffer.cpp ringbuffer.h
        asynclink.h asynclink.cpp
        "${non_embedded_sources}"
)
# asynclink is intended for embedded, but am developing it on desktop for now
# if asynclink is built for pico, it pulls in some pico SDK:
if(PICO)
    target_link_libraries(parachutedev pico_sync pico_stdlib)
endif(PICO)

# Some test harnesses and a link adapter for the pico
if (PICO)
    add_executable(picolinkharness picolinkharness.cpp getline.h getline.cpp)
    target_link_libraries(picolinkharness parachutedev)
    pico_enable_stdio_usb(picolinkharness 1)
    pico_enable_stdio_uart(picolinkharness 1)
    pico_add_extra_outputs(picolinkharness)

    add_executable(picolinkreflect picolinkreflect.cpp debouncer.h debouncer.cpp)
    target_link_libraries(picolinkreflect parachutedev pico_rand)
    pico_enable_stdio_usb(picolinkreflect 1)
    pico_enable_stdio_uart(picolinkreflect 1)
    pico_add_extra_outputs(picolinkreflect)
endif(PICO)

if(NOT(EMBEDDED))
  add_library(testfixtures STATIC tempfilesfixture.h tempfilesfixture.cpp exceptionfixture.h)
  target_link_libraries(testfixtures parachutedev)

  add_executable(testtypes testtypes.cpp)
  target_link_libraries(testtypes gtest gmock_main)
  add_test(NAME testtypes COMMAND testtypes)

  add_executable(testlink testlink.cpp)
  target_link_libraries(testlink parachutedev gtest gmock_main)
  add_test(NAME testlink COMMAND testlink)

  add_executable(testasynclink testasynclink.cpp)
  target_link_libraries(testasynclink parachutedev gtest gmock_main)
  add_test(NAME testasynclink COMMAND testasynclink)

  add_executable(testfilesystem testfilesystem.cpp)
  target_link_libraries(testfilesystem testfixtures parachutedev gtest gmock_main gsl)
  add_test(NAME testfilesystem COMMAND testfilesystem)

  add_executable(testmisc testmisc.cpp)
  target_link_libraries(testmisc parachutedev gtest gmock_main)
  add_test(NAME testmisc COMMAND testmisc)

  add_executable(testcharacterisation testcharacterisation.cpp)
  target_link_libraries(testcharacterisation testfixtures gtest gmock_main)
  add_test(NAME testcharacterisation COMMAND testcharacterisation)
  
  add_executable(testringbuffer testringbuffer.cpp)
  target_link_libraries(testringbuffer parachutedev gtest gmock_main)
  add_test(NAME testringbuffer COMMAND testringbuffer)

endif() # NOT EMBEDDED

