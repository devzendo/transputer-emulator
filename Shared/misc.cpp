//------------------------------------------------------------------------------
//
// File        : misc.cpp
// Description : Miscellaneous functions and stuff
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 15/07/2005
//
// (C) 2005-2020 Matt J. Gumbley
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


