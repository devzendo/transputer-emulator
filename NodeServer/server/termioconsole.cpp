//------------------------------------------------------------------------------
//
// File        : termioconsole.cpp
// Description : The termio (OSX/Linux) Console
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
#include "termioconsole.h"
#include "log.h"

TermioConsole::TermioConsole() : Console() {
    logDebug("Constructing termio console");
}

void TermioConsole::initialise(void) throw (exception) {
    logDebug("Initialising termio console");
}

TermioConsole::~TermioConsole() {
    logDebug("Destroying termio Console");
}

