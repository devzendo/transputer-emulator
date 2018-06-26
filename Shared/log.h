//------------------------------------------------------------------------------
//
// File        : log.h
// Description : simple logging functions
// License     : GNU GPL - see COPYING for more details
// Created     : 06/07/2005
// Revision    : $Revision $
//
// (C) 2005 Matt J. Gumbley
// matt@gumbley.me.uk
// http://www.gumbley.me.uk/parachute
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

extern void _logDebug(int line, char *file, char *s);
extern void _logDebugF(int line, char *file, char *fmt, ...);

extern void logFormat(int level, char *fmt, ...);
extern void logInfo(char *s);
extern void logWarn(char *s);
extern void logError(char *s);
extern void logFatal(char *s);
extern void logBug(char *s);

#define logInfoF(fmt, ...) logFormat(LOGLEVEL_INFO, fmt, __VA_ARGS__)
#define logWarnF(fmt, ...) logFormat(LOGLEVEL_WARN, fmt, __VA_ARGS__)
#define logErrorF(fmt, ...) logFormat(LOGLEVEL_ERROR, fmt, __VA_ARGS__)
#define logFatalF(fmt, ...) logFormat(LOGLEVEL_FATAL, fmt, __VA_ARGS__)

extern void logPrompt(void);
extern void getInput(char *buf, int buflen);

#endif // _LOG_H

