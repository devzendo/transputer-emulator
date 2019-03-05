//------------------------------------------------------------------------------
//
// File        : platform.h
// Description : Centralised platform detection
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 04/03/2019
//
// (C) 2005-2019 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _PLATFORM_H
#define _PLATFORM_H

// See http://nadeausoftware.com/articles/2012/01/c_c_tip_how_use_compiler_predefined_macros_detect_operating_system
// for detail on how to accurately detect which platform we're on. This header encapsulates all that, and gives
// PLATFORM_XXXXX definitions that are used throughout. If new platforms/variants are needed, they can be added here.

// Also useful: https://stackoverflow.com/questions/2989810/which-cross-platform-preprocessor-defines-win32-or-win32-or-win32


// Windows

#if defined(_WIN64)
/* Microsoft Windows (64-bit). ------------------------------ */
#define PLATFORM_WINDOWS
#define PLATFORM_BITS_64

#elif defined(_WIN32)
/* Microsoft Windows (32-bit). ------------------------------ */
#define PLATFORM_WINDOWS
#define PLATFORM_BITS_32

#endif


// OSX

#if defined(__APPLE__) && defined(__MACH__)
/* Apple OSX and iOS (Darwin). ------------------------------ */
#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR == 1
/* iOS in Xcode simulator */
#error iOS Simulator not supported
#elif TARGET_OS_IPHONE == 1
/* iOS on iPhone, iPad, etc. */
#error iOS not supported
#elif TARGET_OS_MAC == 1
/* OSX */
#define PLATFORM_OSX
#define PLATFORM_BITS_64

#endif
#endif


// Linux

#if defined(__linux__)
/* Linux. --------------------------------------------------- */
#define PLATFORM_LINUX

#endif


#endif // _PLATFORM_H

