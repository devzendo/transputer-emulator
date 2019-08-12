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

add_executable(iserver iserver.cpp platform.cpp platform.h platformfactory.h platformfactory.cpp "${platform_sources}")