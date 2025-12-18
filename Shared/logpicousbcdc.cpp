//------------------------------------------------------------------------------
//
// File        : logpicousbcdc.cpp
// Description : logging subsystem - using Pi Pico USB CDC.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 15/12/2025
//
// (C) 2005-2025 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

// #include <cstdlib>
// #include <cstdarg> // vsnprintf is supposed to be in here, but is in cstdio in RH73
#include <mutex> // For std::lock_guard and BasicLockable

#include "log.h"
#include "logbase.h"
#include "cdc_app.h"
#include "sync.h"


void logFlush() {
	LOGMUTEX
	// TODO
}


// Internal. Precondition: LOGMUTEX held by the caller, so allows reentrancy.
void _logLevel(const int level, const char *s) {
	if (myLogLevel > level) {
		return;
	}
	// TODO
}

void _logDebug(int l, const char *f, const char *s) {
	LOGMUTEX
	if (myLogLevel > LOGLEVEL_DEBUG) {
		return;
	}
	// TODO
}

void _logDebugF(int l, const char *f, const char *fmt, ...) {
	LOGMUTEX
	if (myLogLevel > LOGLEVEL_DEBUG) {
		return;
	}
	char *buf;
	int size = 100;
	va_list ap;
	if ((buf = static_cast<char*>(malloc(size))) == nullptr) {
		_logLevel(LOGLEVEL_ERROR, "Out of memory in _logDebugF");
		return;
	}
	while (true) {
		// try to print in allocated buffer
		va_start(ap, fmt);
		const int n = vsnprintf(buf, size, fmt, ap);
		va_end(ap);
		// if ok, display it
		if (n >= -1 && n < size) {
			// TODO
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
			_logLevel(LOGLEVEL_ERROR, "Reallocation failure in _logDebugF");
			free(buf);
			return;
		}
		buf = newbuf;
	}
}


void logBug(const char *s) {
	LOGMUTEX
	// TODO
}

void logPrompt() {
	LOGMUTEX
	// TODO
}

// TODO Isn't this desktop only?
void getInput(char *buf, int buflen) {
	// TODO
}

