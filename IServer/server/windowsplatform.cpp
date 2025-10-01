//------------------------------------------------------------------------------
//
// File        : windowsplatform.cpp
// Description : The Windows Console/Timer
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 05/03/2019
//
// (C) 2005-2025 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN 1
#include <conio.h>
#include <exception>
#include <stdexcept>
// Thank you https://stackoverflow.com/questions/49001326/convert-the-linux-open-read-write-close-functions-to-work-on-windows
#include <io.h>
#include "types.h"
#include "constants.h"
#include "platform.h"
#include "windowsplatform.h"
#include "log.h"

WindowsPlatform::WindowsPlatform() : Platform() {
    logDebug("Constructing Windows platform");
}

void WindowsPlatform::initialise(void) noexcept(false) {
}

WindowsPlatform::~WindowsPlatform() {
    logDebug("Destroying Windows platform");
}

bool WindowsPlatform::isConsoleCharAvailable() {
    return _kbhit();
}

BYTE8 WindowsPlatform::getConsoleChar() {
    return _getch();
}

void WindowsPlatform::putConsoleChar(BYTE8 const ch) {
    _putch(ch);
}


WORD32 WindowsPlatform::getTimeMillis() {
    return 0;
}

UTCTime WindowsPlatform::getUTCTime() {
    return UTCTime(0, 0, 0, 0, 0, 0, 0);
}
