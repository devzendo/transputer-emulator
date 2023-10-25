if(WIN32)
    set(platform_sources namedpipelink.cpp namedpipelink.h)
endif(WIN32)
if(UNIX AND NOT(PICO))
    set(platform_sources fifolink.cpp fifolink.h)
endif(UNIX AND NOT(PICO))
if(PICO)
    # TEST
    set(platform_sources fifolink.cpp fifolink.h)
    #set(platform_sources picousbseriallink.cpp picousbseriallink.h)
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
        "${non_embedded_sources}"
)

if(NOT(EMBEDDED))
  add_library(testfixtures STATIC tempfilesfixture.h tempfilesfixture.cpp exceptionfixture.h)
  target_link_libraries(testfixtures parachutedev)

  add_executable(testtypes testtypes.cpp)
  target_link_libraries(testtypes gtest gmock_main)
  add_test(NAME testtypes COMMAND testtypes)

  add_executable(testlink testlink.cpp)
  target_link_libraries(testlink parachutedev gtest gmock_main)
  add_test(NAME testlink COMMAND testlink)

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

