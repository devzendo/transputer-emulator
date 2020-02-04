# set(CMAKE_VERBOSE_MAKEFILE ON)
cmake_minimum_required(VERSION 3.10.2)
# Thanks to https://crascit.com/2015/03/28/enabling-cxx11-in-cmake/
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON) # Don't fall back to an earlier standard if the one declared isn't available
set(CMAKE_CXX_EXTENSIONS OFF)

# VERSION is filtered into target/classes/version.cpp by using the maven resources plugin.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEBUG -DVERSION=\"${VERSION}\"")
if(WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2 /Zi")
endif(WIN32)

if(UNIX)
  #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -Wall -O2 -g")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -g")
endif(UNIX)

project(transputer-emulator)

########################################################################################################################
# Build googletest & googlemock
# Thanks to https://crascit.com/2015/07/25/cmake-gtest/
# Download and unpack googletest at configure time
configure_file(CMakeLists.googletest.txt googletest-download/CMakeLists.txt)
execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download" )
execute_process(COMMAND "${CMAKE_COMMAND}" --build .
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download" )

# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This adds
# the following targets: gtest, gtest_main, gmock
# and gmock_main
add_subdirectory("${CMAKE_BINARY_DIR}/googletest-src"
        "${CMAKE_BINARY_DIR}/googletest-build")

# The gtest/gmock targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if(CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include"
            "${gmock_SOURCE_DIR}/include")
endif()

# Now simply link your own targets against gtest, gmock,
# etc. as appropriate

include(CTest)
enable_testing()

########################################################################################################################
# Build gsl-lite
# Thanks to 'As external Git project' at https://github.com/gsl-lite/gsl-lite#as-external-git-project
project( use-gsl-lite LANGUAGES CXX )

# Set default ExternalProject root directory and add gsl-lite:
set( GSL_LITE_URL https://github.com/gsl-lite/gsl-lite.git )

include( ExternalProject )
find_package( Git REQUIRED )
set_directory_properties( PROPERTIES EP_PREFIX ${CMAKE_BINARY_DIR}/3rd_party )
ExternalProject_Add(
        gsl-extern
        GIT_REPOSITORY ${GSL_LITE_URL}
        TIMEOUT 10
        UPDATE_COMMAND ${GIT_EXECUTABLE} pull
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        LOG_DOWNLOAD ON
)

# Provide #include access to gsl-lite as <gsl/gsl-lite.hpp>:
ExternalProject_Get_Property( gsl-extern SOURCE_DIR )
set( GSL_LITE_INCLUDE_DIR ${SOURCE_DIR}/include CACHE INTERNAL "Include folder for gsl-lite" )

add_library( gsl INTERFACE )
target_include_directories( gsl INTERFACE ${GSL_LITE_INCLUDE_DIR} )

# Now link your targets with gsl:
# e.g. target_link_libraries(testfilesystem parachutedev gtest gmock_main gsl)


########################################################################################################################
# Build the transputer emulator and related code...
add_subdirectory(target/classes)
add_subdirectory(Shared)
add_subdirectory(IServer)
add_subdirectory(Emulator)
