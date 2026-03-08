//------------------------------------------------------------------------------
//
// File        : misc.cpp
// Description : Miscellaneous functions and stuff
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 15/07/2005
//
// (C) 2005-2026 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <stdexcept>
#include <sstream>

#include "types.h"
#include "misc.h"

#include <algorithm>

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
    return std::string("Unknown error");
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

void _reverse(char *buf) {
    int length = strlen(buf);
    int start = 0;
    int end = length - 1;
    while (start < end) {
        char temp = buf[start];
        buf[start] = buf[end];
        buf[end] = temp;
        end--;
        start++;
    }
}

const char *int_to_ascii(int x) {
    static char buffer[17]; // -2,147,483,648 to 2,147,483,647
    int i = 0;
    if (x == 0) {
        buffer[i++] = '0';
        buffer[i++] = '\0';
        return buffer;
    }
    bool neg;
    if (x < 0) {
        neg = true;
        x = -x;
    } else {
        neg = false;
    }
    while (x != 0) {
        int rem = x % 10;
        buffer[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        x = x / 10;
    }
    if (neg) {
        buffer[i++] = '-';
    }
    buffer[i++] = '\0';
    _reverse(buffer);
    return buffer;
}