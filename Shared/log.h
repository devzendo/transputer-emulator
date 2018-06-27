//------------------------------------------------------------------------------
//
// File        : log.h
// Description : simple logging functions
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 06/07/2005
//
// (C) 2005-2018 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _LOG_H
#define _LOG_H

const int LOGLEVEL_DEBUG = 0;
const int LOGLEVEL_INFO = 1;
const int LOGLEVEL_WARN = 2;
const int LOGLEVEL_ERROR = 3;
const int LOGLEVEL_FATAL = 4;
extern void setLogLevel(int l);
#define logDebug(s) _logDebug(__LINE__, "" __FILE__, s)
// info cpp gave info on varargs in macros
#define logDebugF(fmt, ...) _logDebugF(__LINE__, "" __FILE__, fmt, __VA_ARGS__)

extern void _logDebug(int line, const char *file, const char *s);
extern void _logDebugF(int line, const char *file, const char *fmt, ...);

extern void logFormat(int level, const char *fmt, ...);
extern void logInfo(const char *s);
extern void logWarn(const char *s);
extern void logError(const char *s);
extern void logFatal(const char *s);
extern void logBug(const char *s);

#define logInfoF(fmt, ...) logFormat(LOGLEVEL_INFO, fmt, __VA_ARGS__)
#define logWarnF(fmt, ...) logFormat(LOGLEVEL_WARN, fmt, __VA_ARGS__)
#define logErrorF(fmt, ...) logFormat(LOGLEVEL_ERROR, fmt, __VA_ARGS__)
#define logFatalF(fmt, ...) logFormat(LOGLEVEL_FATAL, fmt, __VA_ARGS__)

extern void logPrompt(void);
extern void getInput(char *buf, int buflen);

#endif // _LOG_H

