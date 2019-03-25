//------------------------------------------------------------------------------
//
// File        : windowsplatform.cpp
// Description : The Windows Console/Timer
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 05/03/2019
//
// (C) 2005-2019 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

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

void WindowsPlatform::initialise(void) throw (std::exception) {
}

WindowsPlatform::~WindowsPlatform() {
    logDebug("Destroying Windows platform");
}

bool WindowsPlatform::isConsoleCharAvailable() {
    return false;
}

BYTE WindowsPlatform::getConsoleChar() {
    return 0;
}

void WindowsPlatform::putConsoleChar(BYTE const ch) {
    // TODO is there a setvbuf for windows??
    // TODO might be better to setvbuf on stdout, and undo this on terminate. Write there?
    fputc(ch, stderr);
}


WORD32 WindowsPlatform::getTimeMillis() {
    return 0;
}

UTCTime WindowsPlatform::getUTCTime() {
    return UTCTime(0, 0, 0, 0, 0, 0, 0);
}
