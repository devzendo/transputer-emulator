//------------------------------------------------------------------------------
//
// File        : windowsconsole.cpp
// Description : The Windows Console
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 05/03/2019
//
// (C) 2005-2019 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include "platform.h"

#include <exception>
#include <stdexcept>
using namespace std;
#include "types.h"
#include "constants.h"
#include "console.h"
#include "windowsconsole.h"
#include "log.h"

WindowsConsole::WindowsConsole() : Console() {
    logDebug("Constructing Windows console");
}

void WindowsConsole::initialise(void) throw (exception) {
}

WindowsConsole::~WindowsConsole() {
    logDebug("Destroying Windows Console");
}

bool WindowsConsole::isCharAvailable() {
    return false;
}

BYTE WindowsConsole::getChar() {
    return 0;
}
