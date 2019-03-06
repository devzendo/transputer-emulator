//------------------------------------------------------------------------------
//
// File        : platform.cpp
// Description : Abstract base class for Console, Timer
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
#include "platformdetection.h"
#include "platform.h"
#include "log.h"

Platform::Platform() {
    bDebug = false;
}

void Platform::initialise(void) throw (exception) {
}

Platform::~Platform(void) {
}

void Platform::setDebug(bool newDebug) {
    bDebug = newDebug;
}

bool Platform::isCharAvailable() {
    return true;
}

BYTE Platform::getChar() {
    return 0;
}