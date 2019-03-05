include_directories(../../Shared)
include_directories(../../target/classes)
include_directories(..)
link_libraries(parachutedev)
link_libraries(parachuteversion)

if(WIN32)
    set(platform_sources windowsconsole.cpp windowsconsole.h)
endif(WIN32)
if(UNIX)
    set(platform_sources termioconsole.cpp termioconsole.h)
endif(UNIX)

add_executable(nodeserver nodeserver.cpp console.cpp console.h consolefactory.h consolefactory.cpp "${platform_sources}")