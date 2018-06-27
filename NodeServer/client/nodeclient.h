//------------------------------------------------------------------------------
//
// File        : nodeclient.h
// Description : library of functions that encapsulate the node server
//               protocol, for client code to use.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 06/09/2005
//
// (C) 2005-2018 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _NODECLIENT_H
#define _NODECLIENT_H

#include "types.h"

//
extern WORD32 getServerVersion(void);
extern void exitServer(void);
//
extern void putConsoleChar(char b);
extern void putConsolePString(BYTE len, char *data);
extern void putConsoleCString(char *data);
extern bool isConsolePutAvailable(void);
extern bool isConsoleGetAvailable(void);
extern char getConsoleChar(void);
//
struct timeGetUTC {
	WORD32 dayInMonth;
	WORD32 monthInYear;
	WORD32 year;
	WORD32 hourInDay;
	WORD32 minuteInHour;
	WORD32 secondInMinute;
	WORD32 millisInSecond;
};


extern WORD32 getTimeMillis(void);
extern void getTimeUTC(struct timeGetUTC *fill);

#endif // _NODECLIENT_H

