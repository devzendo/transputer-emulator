//------------------------------------------------------------------------------
//
// File        : platformfactory.cpp
// Description : Factory for creating derived classes of Platform
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 05/03/2019
//
// (C) 2005-2023 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <cstdlib>
#include <cctype>
#include <cstring>
#include "platformdetection.h"
#include "types.h"
#include "constants.h"
#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
#include "posixplatform.h"
#endif
#if defined(PLATFORM_WINDOWS)
#include "windowsplatform.h"
#endif
#include "platform.h"
#include "platformfactory.h"
#include "log.h"

PlatformFactory::PlatformFactory(bool isDebug) {
    logDebug("PlatformFactory CTOR");
    bDebug = isDebug;
}


Platform *PlatformFactory::createPlatform() {
    Platform *newPlatform = NULL;
#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
    newPlatform = new POSIXPlatform();
#elif defined(PLATFORM_WINDOWS)
    newPlatform = new WindowsPlatform();
#endif
    return newPlatform;
}


