//------------------------------------------------------------------------------
//
// File        : log.cpp
// Description : logging subsystem
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 07/07/2005
//
// (C) 2005-2023 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <cstdlib>
#include <cstdarg> // vsnprintf is supposed to be in here, but is in cstdio in RH73
#include <cstdio>
#ifdef DESKTOP
#include <iostream>
#include <fstream>
#endif 

#include "log.h"

static const char *tags[5] = {
	"DEBUG ", "INFO  ", "WARN  ", "ERROR ", "FATAL ",
};

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

void logFlush(void) {
#ifdef DESKTOP
    myOutputStream->flush();
#endif
}

static int myLogLevel = LOGLEVEL_INFO;
void setLogLevel(int l) {
	myLogLevel = l;
}

void _logDebug(int l, const char *f, const char *s) {
	if (myLogLevel <= LOGLEVEL_DEBUG) {
#ifdef DESKTOP
		*myOutputStream << tags[LOGLEVEL_DEBUG] << f << ":" << l << " " << s << std::endl;
#elif EMBEDDED
		// TODO
#endif
	}
}
void _logDebugF(int l, const char *f, const char *fmt, ...) {
	char *buf;
	va_list ap;
	int n, size = 100;
	if (myLogLevel <= LOGLEVEL_DEBUG) {
		if ((buf = (char *)malloc(size)) == NULL) {
			logError("Out of memory in _logDebugF");
			return;
		}
		while (1) {
			// try to print in allocated buffer
			va_start(ap, fmt);
			n = vsnprintf(buf, size, fmt, ap);
			va_end(ap);
			// if ok, display it
			if (n >= -1 && n < size) {
#ifdef DESKTOP
				*myOutputStream << tags[LOGLEVEL_DEBUG] << f << ":" << l << " " << buf << std::endl;
#elif EMBEDDED
				// TODO
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
			if ((buf = (char *)realloc(buf, size)) == NULL) {
				logError("Reallocation failure in _logDebugF");
				return;
			}
		}
	}
}

void logFormat(int level, const char *fmt, ...) {
	char *buf;
	va_list ap;
	int n, size = 100;
	if (myLogLevel <= level) {
		if ((buf = (char *)malloc(size)) == NULL) {
			logError("Out of memory in logFormat");
			return;
		}
		while (1) {
			// try to print in allocated buffer
			va_start(ap, fmt);
			n = vsnprintf(buf, size, fmt, ap);
			va_end(ap);
			// if ok, return it - caller must free it
			if (n >= -1 && n < size) {
#ifdef DESKTOP
				*myOutputStream << tags[level] << buf << std::endl;
#elif EMBEDDED
				// TODO
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
			if ((buf = (char *)realloc(buf, size)) == NULL) {
				logError("Reallocation failure in logFormat");
				return;
			}
		}
	}
}

void logInfo(const char *s) {
	if (myLogLevel <= LOGLEVEL_INFO) {
#ifdef DESKTOP
		*myOutputStream << tags[LOGLEVEL_INFO] << s << std::endl;
#elif EMBEDDED
		// TODO
#endif
	}
}

void logWarn(const char *s) {
	if (myLogLevel <= LOGLEVEL_WARN) {
#ifdef DESKTOP
		*myOutputStream << tags[LOGLEVEL_WARN] << s << std::endl;
#elif EMBEDDED
		// TODO
#endif
	}
}

void logError(const char *s) {
	if (myLogLevel <= LOGLEVEL_ERROR) {
#ifdef DESKTOP
		*myOutputStream << tags[LOGLEVEL_ERROR] << s << std::endl;
#elif EMBEDDED
		// TODO
#endif

	}
}

void logFatal(const char *s) {
	if (myLogLevel <= LOGLEVEL_FATAL) {
#ifdef DESKTOP
		*myOutputStream << tags[LOGLEVEL_FATAL] << s << std::endl;
#elif EMBEDDED
		// TODO
#endif
	}
}

void logBug(const char *s) {
#ifdef DESKTOP
	*myOutputStream << "*BUG* " << s << std::endl;
#elif EMBEDDED
	// TODO
#endif
}

#ifdef DESKTOP
void logPrompt(void) {
	*myOutputStream << "> ";
	myOutputStream->flush();
}

void getInput(char *buf, int buflen) {
	if (fgets(buf, buflen, stdin) == NULL) {
		// do nothing. casting fgets' output to void still causes warnings
	}
}
#endif // DESKTOP

#ifdef EMBEDDED
static RingBuffer *logRingBuffer = NULL;
void setLogRingBuffer(RingBuffer *buf) {
	logRingBuffer = buf;
}

#endif // EMBEDDED

