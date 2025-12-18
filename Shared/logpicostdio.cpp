//------------------------------------------------------------------------------
//
// File        : logpicostdio.cpp
// Description : logging subsystem - using Pi Pico stdio.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 15/12/2025
//
// (C) 2005-2025 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <pico/stdio.h> // Pico USB Serial STDIO
//#include <cstdlib>
#include <cstdarg> // vsnprintf is supposed to be in here, but is in cstdio in RH73
#include <mutex> // For std::lock_guard and BasicLockable

#include "log.h"
#include "logbase.h"
#include "sync.h"

/* Access to the log buffer is protected by a std::lock_guard
 * and an appropriate lock for the platform - std::mutex on desktop, and a critical_section_t
 * on PICO, wrapped in the CriticalSection BasicResolvable.
 */

void logFlush() {
	LOGMUTEX
	fflush(stdout);
	stdio_flush();
}


// Internal. Precondition: LOGMUTEX held by the caller, so allows reentrancy.
void _logLevel(const int level, const char *s) {
	if (myLogLevel > level) {
		return;
	}
	fputs(tags[level], stdout);
	fputs(s, stdout);
	fputs("\r\n", stdout);
	stdio_flush();
}

void _logDebug(int l, const char *f, const char *s) {
	LOGMUTEX
	if (myLogLevel > LOGLEVEL_DEBUG) {
		return;
	}
	fputs(tags[LOGLEVEL_DEBUG], stdout);
	//fputs(f, stdout);
	//putchar(':');
	//printf("%d ", l);
	fputs(s, stdout);
	fputs("\n", stdout);
	stdio_flush();
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
			fputs(tags[LOGLEVEL_DEBUG], stdout);
			//fputs(f, stdout);
			//putchar(':');
			//printf("%d ", l);
			fputs(buf, stdout);
			fputs("\r\n", stdout);
			stdio_flush();
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
	fputs("*BUG* ", stdout);
	fputs(s, stdout);
	fputs("\r\n", stdout);
	stdio_flush();
}

void logPrompt() {
	LOGMUTEX
	stdio_put_string("> ", 2, false, false);
	stdio_flush();
}

// TODO Isn't this desktop only?
void getInput(char *buf, int buflen) {
	if (fgets(buf, buflen, stdin) == nullptr) {
		// do nothing. casting fgets output to void still causes warnings
	}
}

