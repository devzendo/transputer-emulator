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

#include "types.h"
#include "constants.h"
#include "platformdetection.h"
#include "platform.h"
#include "log.h"

Platform::Platform() {
    bDebug = false;
}

Platform::~Platform(void) {
}

void Platform::setDebug(bool newDebug) {
    bDebug = newDebug;
}
