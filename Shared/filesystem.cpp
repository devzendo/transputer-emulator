//------------------------------------------------------------------------------
//
// File        : filesystem.cpp
// Description : Portable filesystem routines
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 28/01/2020
//
// (C) 2005-2020 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include "filesystem.h"
#include "platformdetection.h"

#if defined(PLATFORM_WINDOWS)
#include <windows.h>
#include <tchar.h>
#endif

#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
#include <sys/stat.h>
#include <errno.h>
#endif


namespace {
#if defined(PLATFORM_WINDOWS)
#endif

#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
    const char *default_tmp = "/tmp";
#endif
}


// What is the system temporary directory?
// TODO C++17: replace this with std::filesystem::temp_directory_path
// Thanks to https://stackoverflow.com/questions/8087805/how-to-get-system-or-user-temp-folder-in-unix-and-windows
std::string tempdir() {
    static char msgbuf[255];

#if defined(PLATFORM_WINDOWS)
    DWORD dwRetVal = 0;

    //  Gets the temp path env string (no guarantee it's a valid path).
    dwRetVal = GetTempPath(MAX_PATH,          // length of the buffer
                           lpTempPathBuffer); // buffer for path
    if (dwRetVal > MAX_PATH || (dwRetVal == 0))
    {
        sprintf(msgbuf, "Could not obtain the temp directory via GetTempPath", path, strerror(errno));
        throw std::runtime_error(msgbuf);
        PrintError(TEXT("GetTempPath failed"));
        return (2);
    }

#endif

#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
    const char *val = nullptr;

    (val = std::getenv("TMPDIR" )) ||
    (val = std::getenv("TMP"    )) ||
    (val = std::getenv("TEMP"   )) ||
    (val = std::getenv("TEMPDIR"));

    const char *path((val != nullptr) ? val : default_tmp);

    struct stat statBuffer{};
    if (stat(path, &statBuffer) != 0) {
        sprintf(msgbuf, "Could not obtain file status of temp directory %s: %s", path, strerror(errno));
        throw std::runtime_error(std::string("Could not obtain file status of temp directory ") + path + ": " + strerror(errno));
    }
    if ((statBuffer.st_mode & S_IFDIR) == 0) {
        sprintf(msgbuf, "The 'temp directory' %s is not a directory", path);
        throw std::runtime_error(msgbuf);
    }

    return std::string(path);
#endif
}

