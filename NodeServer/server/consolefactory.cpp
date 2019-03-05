//------------------------------------------------------------------------------
//
// File        : consolefactory.cpp
// Description : Factory for creating derived classes of Console
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 05/03/2019
//
// (C) 2005-2019 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <cstdlib>
#include <cctype>
#include <cstring>
#include "platform.h"
#include "types.h"
#include "constants.h"
#include "console.h"
#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
#include "termioconsole.h"
#endif
#if defined(PLATFORM_WINDOWS)
#include "windowsconsole.h"
#endif
#include "consolefactory.h"
#include "log.h"

ConsoleFactory::ConsoleFactory(bool isDebug) {
    logDebug("ConsoleFactory CTOR");
    bDebug = isDebug;
}


Console *ConsoleFactory::createConsole() {
    Console *newConsole = NULL;
#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
    newConsole = new TermioConsole();
#elif defined(PLATFORM_WINDOWS)
    newConsole = new WindowsConsole();
#endif
    return newConsole;
}


