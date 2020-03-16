if(WIN32)
    set(platform_sources namedpipelink.cpp namedpipelink.h)
endif(WIN32)
if(UNIX)
    set(platform_sources fifolink.cpp fifolink.h)
endif(UNIX)
add_library(parachutedev STATIC log.h log.cpp misc.h misc.cpp hexdump.h hexdump.cpp link.h link.cpp
        "${platform_sources}"
        linkfactory.h linkfactory.cpp constants.h emuquery.h memloc.h platformdetection.h types.h
        stublink.cpp stublink.h
        filesystem.cpp filesystem.h)

add_library(testshared STATIC testtempfiles.h testtempfiles.cpp testexception.h)
target_link_libraries(testshared parachutedev)

add_executable(testtypes testtypes.cpp)
target_link_libraries(testtypes gtest gmock_main)
add_test(NAME testtypes COMMAND testtypes)

add_executable(testlink testlink.cpp)
target_link_libraries(testlink parachutedev gtest gmock_main)
add_test(NAME testlink COMMAND testlink)

add_executable(testfilesystem testfilesystem.cpp)
target_link_libraries(testfilesystem testshared parachutedev gtest gmock_main gsl)
add_test(NAME testfilesystem COMMAND testfilesystem)

add_executable(testmisc testmisc.cpp)
target_link_libraries(testmisc parachutedev gtest gmock_main)
add_test(NAME testmisc COMMAND testmisc)
