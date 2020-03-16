include_directories(../../Shared)
include_directories(../../target/classes)
include_directories(..)
link_libraries(parachutedev)
link_libraries(parachuteversion)

if(WIN32)
    set(platform_sources windowsplatform.cpp windowsplatform.h)
endif(WIN32)
if(UNIX)
    set(platform_sources posixplatform.cpp posixplatform.h)
endif(UNIX)

add_executable(iserver iserver.cpp platform.cpp platform.h platformfactory.h platformfactory.cpp framecodec.cpp framecodec.h protocolhandler.cpp protocolhandler.h "${platform_sources}")

add_executable(testprotocol testprotocol.cpp framecodec.cpp framecodec.h protocolhandler.cpp protocolhandler.h memstreambuf.h ../../Shared/link.cpp platform.cpp)
target_link_libraries(testprotocol testshared gtest gmock_main)
add_test(NAME testprotocol COMMAND testprotocol)

add_executable(testframecodec testframecodec.cpp framecodec.cpp framecodec.h)
target_link_libraries(testframecodec gtest gmock_main)
add_test(NAME testframecodec COMMAND testframecodec)

add_executable(testplatform testplatform.cpp platform.cpp platform.h platformfactory.cpp platformfactory.h memstreambuf.h "${platform_sources}")
target_link_libraries(testplatform testshared gtest gmock_main)
add_test(NAME testplatform COMMAND testplatform)
