//------------------------------------------------------------------------------
//
// File        : filesystem.cpp
// Description : Portable filesystem routines
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 28/01/2020
//
// (C) 2005-2024 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include "filesystem.h"
#include "platformdetection.h"
#include "misc.h"

#if defined(PLATFORM_WINDOWS)
#include <windows.h>
#include <tchar.h>
#include <sys/types.h>
#endif

#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
#include <string.h>
#include <errno.h>
#endif

#include <sys/stat.h>

namespace {
#if defined(PLATFORM_WINDOWS)
#endif

#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
    const char *default_tmp = "/tmp";
#endif

}

#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
const char pathSeparator = '/';
#endif
#if defined(PLATFORM_WINDOWS)
const char pathSeparator = '\\';
#endif


// What is the system temporary directory?
// TODO C++17: replace this with std::filesystem::temp_directory_path
// Thanks to https://stackoverflow.com/questions/8087805/how-to-get-system-or-user-temp-folder-in-unix-and-windows
std::string tempdir() {

#if defined(PLATFORM_WINDOWS)
    DWORD dwRetVal = 0;
    TCHAR pathBuf[MAX_PATH];

    // Gets the temp path env string (no guarantee it's a valid path).
    dwRetVal = GetTempPath(MAX_PATH, pathBuf);
    if (dwRetVal > MAX_PATH || (dwRetVal == 0))  {
        throw std::runtime_error(Formatter() << "Could not obtain the temporary directory via GetTempPath: " << GetLastErrorStdStr() );
    }

    return std::string(pathBuf);
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
        throw std::runtime_error(Formatter() << "Could not obtain file status of temp directory " << path << ": " << strerror(errno) );
    }
    if ((statBuffer.st_mode & S_IFDIR) == 0) {
        throw std::runtime_error(Formatter() << "The 'temp directory' " << path << " is not a directory");
    }

    return std::string(path);
#endif
}

bool pathIsDir(const std::string &path) {
    int err = 0;
    bool isDir = false;
#if defined(PLATFORM_WINDOWS)
    struct _stat statBuffer{};
    err = _stat(path.c_str(), &statBuffer);
    isDir = (statBuffer.st_mode & _S_IFDIR) == _S_IFDIR;
#endif
#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
    struct stat statBuffer{};
    err = stat(path.c_str(), &statBuffer);
    isDir = (statBuffer.st_mode & S_IFDIR) == S_IFDIR;
#endif
    if (err != 0) {
        throwLastError("Could not obtain status of path '" + path + "': ");
    }
    return isDir;
}

std::string pathJoin(const std::string &lhs, const std::string &rhs) {
    std::string ret;
    ret += stripTrailing(pathSeparator, lhs);
    ret.push_back(pathSeparator);
    ret += stripLeading(pathSeparator, rhs);
    return ret;
}
