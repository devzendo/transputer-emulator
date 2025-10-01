//------------------------------------------------------------------------------
//
// File        : platformdetection.h
// Description : Centralised platform detection
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 04/03/2019
//
// (C) 2005-2025 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _PLATFORMDETECTION_H
#define _PLATFORMDETECTION_H

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

#if defined(PLATFORM_WINDOWS)
// See https://stackoverflow.com/questions/54525846/alternative-to-sscanf-that-makes-both-linux-and-windows-happy
// This should obviate the need for windows-specific sscanf_s idiocy and the like.
// When this symbol is defined, the CRT enables template overloads of standard functions tht coall the more secure
// variants automatically. The compiler also warns that this symbol (macro?) has been redefined.. so suppress that.
#pragma warning(disable : 4005)
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
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

#if defined(__linux__) && !defined EMBEDDED
/* Linux. --------------------------------------------------- */
#define PLATFORM_LINUX

#endif


#if defined(PICO)
#define PLATFORM_PICO
#endif


// Portability for C runtime functions that have different names on different platforms

#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
#define platform_open open
#define platform_read read
#define platform_write write
#define platform_close close
#define platform_unlink unlink
#endif
#if defined(PLATFORM_WINDOWS)
#define platform_open _open
#define platform_read _read
#define platform_write _write
#define platform_close _close
#define platform_unlink _unlink
#endif

#endif // _PLATFORMDETECTION_H

