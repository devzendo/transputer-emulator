//------------------------------------------------------------------------------
//
// File        : misc.cpp
// Description : Miscellaneous functions and stuff
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 15/07/2005
//
// (C) 2005-2025 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <stdexcept>
#include <sstream>

#include "types.h"
#include "misc.h"
#include "log.h"
#include "platformdetection.h"

#if defined(PLATFORM_WINDOWS)
#include <windows.h>
#include <tchar.h>
#endif

#if defined(PLATFORM_WINDOWS)

// Thanks to Orjan Westin
// From https://www.codeproject.com/Tips/479880/GetLastError-as-std-string
// This routine is under the BSD License.

// Create a string with last error message
std::string GetLastErrorStdStr()
{
    DWORD error = GetLastError();
    if (error) {
        LPVOID lpMsgBuf;
        DWORD bufLen = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) &lpMsgBuf,
            0, NULL );

        if (bufLen) {
            LPCSTR lpMsgStr = (LPCSTR)lpMsgBuf;
            std::string result(lpMsgStr, lpMsgStr + bufLen);

            LocalFree(lpMsgBuf);

            return result;
        }
    }
    return std::string();
}

#endif

void throwLastError(const std::string &prefix) {
    throw std::runtime_error(Formatter() << prefix << getLastError());
}

std::string getLastError() {
#if defined(PLATFORM_WINDOWS)
    return GetLastErrorStdStr();
#endif
#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
    return std::string(strerror(errno));
#endif
}


std::string stripTrailing(const char toStrip, const std::string &from) {
    if (from.empty() || from.back() != toStrip) {
        return from;
    }
    else {
        std::string ret(from);
        while (!ret.empty() && ret.back() == toStrip) {
            ret.pop_back();
        }
        return ret;
    }
}


std::string stripLeading(const char toStrip, const std::string &from) {
    if (from.empty() || from.front() != toStrip) {
        return from;
    }
    else {
        std::string ret(from);
        while (!ret.empty() && ret.front() == toStrip) {
            ret.erase(0, 1);
        }
        return ret;
    }
}


const char *byte_to_binary(BYTE8 x) {
    static char buffer[9]; // 8 bits + null terminator
    buffer[0] = '\0';

    int z;
    for (z = 128; z > 0; z >>= 1) {
        strcat(buffer, ((x & z) == z) ? "1" : "0");
    }
    return buffer;
}


const char *word_to_binary(WORD16 x) {
    static char buffer[17]; // 16 bits + null terminator
    buffer[0] = '\0';

    int z;
    for (z = 32768; z > 0; z >>= 1) {
        strcat(buffer, ((x & z) == z) ? "1" : "0");
    }
    return buffer;
}
