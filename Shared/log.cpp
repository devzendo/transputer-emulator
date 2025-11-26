//------------------------------------------------------------------------------
//
// File        : log.cpp
// Description : logging subsystem
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 07/07/2005
//
// (C) 2005-2025 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifdef PICO
#include <cstdlib>
#include <stdio.h> // Pico USB Serial STDIO
#endif

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
#ifdef PICO
	fflush(stdout);
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
#endif
#ifdef PICO
		fputs(tags[LOGLEVEL_DEBUG], stdout);
		//fputs(f, stdout);
		//putchar(':');
		//printf("%d ", l);
		fputs(s, stdout);
		fputs("\n", stdout);
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
#endif
#ifdef PICO
				fputs(tags[LOGLEVEL_DEBUG], stdout);
				//fputs(f, stdout);
				//putchar(':');
				//printf("%d ", l);
				fputs(buf, stdout);
				fputs("\n", stdout);
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
#endif
#ifdef PICO
				fputs(tags[level], stdout);
				fputs(buf, stdout);
				fputs("\n", stdout);
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

void logLevel(const int level, const char *s) {
	if (myLogLevel <= level) {
#ifdef DESKTOP
		*myOutputStream << tags[level] << s << std::endl;
#endif
#ifdef PICO
		fputs(tags[level], stdout);
		fputs(s, stdout);
		fputs("\n", stdout);
#endif
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
#ifdef DESKTOP
	*myOutputStream << "*BUG* " << s << std::endl;
#endif
#ifdef PICO
	fputs("*BUG* ", stdout);
	fputs(s, stdout);
	fputs("\n", stdout);
#endif
}

void logPrompt(void) {
#ifdef DESKTOP
	*myOutputStream << "> ";
	myOutputStream->flush();
#endif
}

// TODO Isn't this desktop only?
void getInput(char *buf, int buflen) {
	if (fgets(buf, buflen, stdin) == NULL) {
		// do nothing. casting fgets' output to void still causes warnings
	}
}

