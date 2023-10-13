//------------------------------------------------------------------------------
//
// File        : log.h
// Description : simple logging functions
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 06/07/2005
//
// (C) 2005-2023 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _LOG_H
#define _LOG_H

#include <cstring>

const int LOGLEVEL_DEBUG = 0;
const int LOGLEVEL_INFO = 1;
const int LOGLEVEL_WARN = 2;
const int LOGLEVEL_ERROR = 3;
const int LOGLEVEL_FATAL = 4;
extern void setLogLevel(int l);

// All logging output goes to std::cout, unless this is called to switch it to a file "test.log" (useful for seeing actual code
// output during Google Test execution which otherwise silences std::cout)
extern void logToTestLog(void);
extern void logToFile(const char *fileName);

extern void logFlush(void);

extern void _logDebug(int line, const char *file, const char *s);
extern void _logDebugF(int line, const char *file, const char *fmt, ...);

// Portable method of getting just the base filename from alexander golks' answer to
// https://stackoverflow.com/questions/8487986/file-macro-shows-full-path
// And works with the Make build too.
// Note that this is not done at compile time.
#define JUSTFILE (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

#define logDebug(s) _logDebug(__LINE__, JUSTFILE, s)
// info cpp gave info on varargs in macros
#define logDebugF(fmt, ...) _logDebugF(__LINE__, JUSTFILE, fmt, __VA_ARGS__)

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

#ifdef DESKTOP
extern void logPrompt(void);
extern void getInput(char *buf, int buflen);
#endif

#endif // _LOG_H

