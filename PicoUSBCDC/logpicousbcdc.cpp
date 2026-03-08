//------------------------------------------------------------------------------
//
// File        : logpicousbcdc.cpp
// Description : logging subsystem - using Pi Pico USB CDC.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 15/12/2025
//
// (C) 2005-2026 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

// #include <cstdlib>
// #include <cstdarg> // vsnprintf is supposed to be in here, but is in cstdio in RH73
#include <mutex> // For std::lock_guard and BasicLockable

#include "../Shared/log.h"
#include "../Shared/logbase.h"

#include "cdc_app.h"
#include "../Shared/sync.h"
#include "../Shared/misc.h"

void logFlush() {
	usb_poll();
	LOGMUTEX
	usb_log_flush();
	usb_poll();
}

void _logLineFlush(const char *s) {
	usb_log_write((void *) s, strlen(s));
	usb_log_write((void *) "\r\n", 2);
	usb_log_flush();
	usb_poll();
}

// Internal. Precondition: LOGMUTEX held by the caller, so allows reentrancy.
void _logLevel(const int level, const char *s) {
	usb_poll();
	if (myLogLevel > level) {
		return;
	}
	usb_log_write((void *) tags[level], g_tag_length);
	_logLineFlush(s);
}

void _logDebug(int l, const char *f, const char *s) {
	usb_poll();
	LOGMUTEX
	if (myLogLevel > LOGLEVEL_DEBUG) {
		return;
	}
	usb_log_write((void *) tags[LOGLEVEL_DEBUG], g_tag_length);
	usb_log_write((void *) f, strlen(f));
	usb_log_write((void *) ":", 1);
	const char *lbuf = int_to_ascii(l);
	usb_log_write((void *) lbuf, strlen(lbuf));
	_logLineFlush(s);
}

void _logDebugF(int l, const char *f, const char *fmt, ...) {
	usb_poll();
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
			usb_log_write((void *) tags[LOGLEVEL_DEBUG], g_tag_length);
			usb_log_write((void *) f, strlen(f));
			usb_log_write((void *) ":", 1);
			const char *lbuf = int_to_ascii(l);
			usb_log_write((void *) lbuf, strlen(lbuf));
			_logLineFlush(buf);
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
	usb_poll();
	LOGMUTEX
	usb_log_write((void *) "*BUG* ", g_tag_length);
	_logLineFlush(s);
}

void logPrompt() {
	usb_poll();
	LOGMUTEX
	usb_log_write((void *) "> ", 2);
	usb_log_flush();
	usb_poll();
}

// TODO Isn't this desktop only?
void getInput(char *buf, int buflen) {
	// TODO
}

