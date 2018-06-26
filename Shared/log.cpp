//------------------------------------------------------------------------------
//
// File        : log.cpp
// Description : logging subsystem
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 07/07/2005
// Revision    : $Revision $
//
// (C) 2005-2018 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <cstdlib>
#include <cstdarg> // vsnprintf is supposed to be in here, but is in cstdio in RH73
#include <cstdio>
#include <iostream>
using namespace std;
#include "log.h"

static const char *tags[5] = {
	"DEBUG ", "INFO  ", "WARN  ", "ERROR ", "FATAL ",
};

static int myLogLevel = LOGLEVEL_INFO;
void setLogLevel(int l) {
	myLogLevel = l;
}

void _logDebug(int l, const char *f, const char *s) {
	if (myLogLevel <= LOGLEVEL_DEBUG) {
		cout << tags[LOGLEVEL_DEBUG] << f << ":" << l << " " << s << endl;
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
				cout << tags[LOGLEVEL_DEBUG] << f << ":" << l << " " << buf << endl;
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
				cout << tags[level] << buf << endl;
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
		cout << tags[LOGLEVEL_INFO] << s << endl;
	}
}

void logWarn(const char *s) {
	if (myLogLevel <= LOGLEVEL_WARN) {
		cout << tags[LOGLEVEL_WARN] << s << endl;
	}
}

void logError(const char *s) {
	if (myLogLevel <= LOGLEVEL_ERROR) {
		cout << tags[LOGLEVEL_ERROR] << s << endl;
	}
}

void logFatal(const char *s) {
	if (myLogLevel <= LOGLEVEL_FATAL) {
		cout << tags[LOGLEVEL_FATAL] << s << endl;
	}
}

void logBug(const char *s) {
	cout << "*BUG* " << s << endl;
}

void logInfoF_old(char *fmt, ...) {
	char *buf;
	va_list ap;
	int n, size = 100;
	if (myLogLevel <= LOGLEVEL_INFO) {
		if ((buf = (char *)malloc(size)) == NULL) {
			logError("Out of memory in logInfoF");
			return;
		}
		while (1) {
			// try to print in allocated buffer
			va_start(ap, fmt);
			n = vsnprintf(buf, size, fmt, ap);
			va_end(ap);
			// if ok, display it
			if (n >= -1 && n < size) {
				cout << "INFO  " << buf << endl;
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
				logError("Reallocation failure in logInfoF");
				return;
			}
		}
	}
}

void logPrompt(void) {
	cout << "> ";
	fflush(stdout);
}

void getInput(char *buf, int buflen) {
	fgets(buf, buflen, stdin);
}

