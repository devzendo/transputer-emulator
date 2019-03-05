//------------------------------------------------------------------------------
//
// File        : console.cpp
// Description : Abstract base class for consoles
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 05/03/2019
//
// (C) 2005-2019 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <exception>
using namespace std;

#include "types.h"
#include "constants.h"
#include "console.h"
#include "log.h"

Console::Console() {
    bDebug = false;
}

void Console::initialise(void) throw (exception) {
}

Console::~Console(void) {
}

void Console::setDebug(bool newDebug) {
    bDebug = newDebug;
}

bool Console::isCharAvailable() {
    return true;
}

BYTE Console::getChar() {
    return 0;
}