//------------------------------------------------------------------------------
//
// File        : logbase.cpp
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

#include "log.h"

static const char *tags[5] = {
	"DEBUG ", "INFO  ", "WARN  ", "ERROR ", "FATAL ",
};

/* Access to the log buffer is protected by a std::lock_guard
 * and an appropriate lock for the platform - std::mutex on desktop, and a critical_section_t
 * on PICO, wrapped in the CriticalSection BasicResolvable.
 */

#ifdef DESKTOP
#define LOGMUTEX std::lock_guard<std::mutex> guard(g_log_mutex);
static std::mutex g_log_mutex;
#endif

#ifdef PICO
#define LOGMUTEX std::lock_guard<CriticalSection> guard(g_log_criticalsection);
static CriticalSection g_log_criticalsection;
#endif

static int myLogLevel = LOGLEVEL_INFO;
void setLogLevel(int l) {
	myLogLevel = l;
}

void logLevel(const int level, const char *s) {
	LOGMUTEX
	_logLevel(level, s);
}

void logFormat(int level, const char *fmt, ...) {
	LOGMUTEX
	if (myLogLevel > level) {
		return;
	}
	char *buf;
	va_list ap;
	int size = 100;
	if ((buf = static_cast<char*>(malloc(size))) == nullptr) {
		_logLevel(LOGLEVEL_ERROR, "Out of memory in logFormat");
		return;
	}
	while (true) {
		// try to print in allocated buffer
		va_start(ap, fmt);
		const int n = vsnprintf(buf, size, fmt, ap);
		va_end(ap);
		// if ok, return it - caller must free it
		if (n >= -1 && n < size) {
			_logLevel(level, buf);
			free(buf);
			return;
		}
		// else try again with more space
		if (n >= -1) { // glibc 2.1
			size = n+1; // precisely what is needed
		} else { // glibc 2.0
			size *= 2; // twice old size
		}
		char *newbuf;
		if ((newbuf = static_cast<char*>(realloc(buf, size))) == nullptr) {
			_logLevel(LOGLEVEL_ERROR, "Reallocation failure in logFormat");
			free(buf);
			return;
		}
		buf = newbuf;
	}
}

void logInfo(const char *s) {
	logLevel(LOGLEVEL_INFO, s);
}

void logWarn(const char *s) {
	logLevel(LOGLEVEL_WARN, s);
}

void logError(const char *s) {
	logLevel(LOGLEVEL_ERROR, s);
}

void logFatal(const char *s) {
	logLevel(LOGLEVEL_FATAL, s);
}
