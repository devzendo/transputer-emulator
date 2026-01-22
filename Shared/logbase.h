//------------------------------------------------------------------------------
//
// File        : logbase.h
// Description : logging subsystem - functions used in all implementations.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 15/12/2025
//
// (C) 2005-2025 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <cstdlib>
#include <cstdarg> // vsnprintf is supposed to be in here, but is in cstdio in RH73
#include <cstdio>
#include <mutex> // For std::lock_guard and BasicLockable

#ifdef DESKTOP
#include <iostream>
#include <fstream>
#endif

#ifdef PICO
#include "sync.h"
#endif

#include "log.h"

extern const char *tags[5];

#ifdef DESKTOP
#define LOGMUTEX std::lock_guard<std::mutex> guard(g_log_mutex);
extern std::mutex g_log_mutex;
#endif

#ifdef PICO
#define LOGMUTEX std::lock_guard<CriticalSection> guard(g_log_criticalsection);
extern CriticalSection g_log_criticalsection;
#endif

extern int myLogLevel;
extern void setLogLevel(int l);
extern void logLevel(const int level, const char *s);
extern void logFormat(int level, const char *fmt, ...);
