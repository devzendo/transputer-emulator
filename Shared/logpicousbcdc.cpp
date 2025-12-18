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

#ifdef PICO
#include "cdc_app.h"
#endif

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

#ifdef DESKTOP
static std::ostream *myOutputStream = &std::cout;
static bool loggingToTestLog = false;
static std::ofstream logFile;


void logToTestLog(void) {
	logToFile("test.log");
}

void logToFile(const char *fileName) {
	if (!loggingToTestLog) {
		logFile.open(fileName, std::ios::out | std::ios::app);
		myOutputStream = &logFile;
		loggingToTestLog = true;
	}
}
#endif // DESKTOP

static int myLogLevel = LOGLEVEL_INFO;
void setLogLevel(int l) {
	myLogLevel = l;
}

void logFlush() {
	LOGMUTEX
#ifdef DESKTOP
    myOutputStream->flush();
#endif
#ifdef PICO
	fflush(stdout);
	stdio_flush();
#endif
}


// Internal. Precondition: LOGMUTEX held by the caller, so allows reentrancy.
void _logLevel(const int level, const char *s) {
	if (myLogLevel > level) {
		return;
	}
#ifdef DESKTOP
	*myOutputStream << tags[level] << s << std::endl;
#endif
#ifdef PICO
	fputs(tags[level], stdout);
	fputs(s, stdout);
	fputs("\r\n", stdout);
	stdio_flush();
#endif
}

void _logDebug(int l, const char *f, const char *s) {
	LOGMUTEX
	if (myLogLevel > LOGLEVEL_DEBUG) {
		return;
	}
#ifdef DESKTOP
	*myOutputStream << tags[LOGLEVEL_DEBUG] << f << ":" << l << " " << s << std::endl;
#endif
#ifdef PICO
	fputs(tags[LOGLEVEL_DEBUG], stdout);
	//fputs(f, stdout);
	//putchar(':');
	//printf("%d ", l);
	fputs(s, stdout);
	fputs("\n", stdout);
	stdio_flush();
#endif
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
#ifdef DESKTOP
			*myOutputStream << tags[LOGLEVEL_DEBUG] << f << ":" << l << " " << buf << std::endl;
#endif
#ifdef PICO
			fputs(tags[LOGLEVEL_DEBUG], stdout);
			//fputs(f, stdout);
			//putchar(':');
			//printf("%d ", l);
			fputs(buf, stdout);
			fputs("\r\n", stdout);
			stdio_flush();
#endif
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

void logBug(const char *s) {
	LOGMUTEX
#ifdef DESKTOP
	*myOutputStream << "*BUG* " << s << std::endl;
#endif
#ifdef PICO
	fputs("*BUG* ", stdout);
	fputs(s, stdout);
	fputs("\r\n", stdout);
	stdio_flush();
#endif
}

void logPrompt() {
	LOGMUTEX
#ifdef DESKTOP
	*myOutputStream << "> ";
	myOutputStream->flush();
#endif
#ifdef PICO
	stdio_put_string("> ", 2, false, false);
	stdio_flush();
#endif
}

// TODO Isn't this desktop only?
void getInput(char *buf, int buflen) {
	if (fgets(buf, buflen, stdin) == nullptr) {
		// do nothing. casting fgets output to void still causes warnings
	}
}

