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
using namespace std;
#include "types.h"
#include "constants.h"
#include "platform.h"
#include "windowsplatform.h"
#include "log.h"

WindowsPlatform::WindowsPlatform() : Platform() {
    logDebug("Constructing Windows platform");
}

void WindowsPlatform::initialise(void) throw (exception) {
}

WindowsPlatform::~WindowsPlatform() {
    logDebug("Destroying Windows platform");
}

bool WindowsPlatform::isCharAvailable() {
    return false;
}

BYTE WindowsPlatform::getChar() {
    return 0;
}
