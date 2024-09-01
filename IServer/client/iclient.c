//------------------------------------------------------------------------------
//
// File        : iclient.cpp
// Description : library of functions that encapsulate the iserver
//               protocol, for client code to use.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 06/09/2005
//
// (C) 2005-2024 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include "types.h"
#include "memloc.h"
#include "isproto.h"
#include "iclient.h"

// TODO rewrite using the iserver protocol, in assembler.

//------------------------------------------------------------------------------
// API functions
//------------------------------------------------------------------------------

WORD32 getServerVersion(void) {
	WORD32 version=0;
	asm __volatile
	("outword\n\t"
	: /* no outputs */
	: "bP" (Link0Output),
	  "aP" (SERVER_GET_VERSION)
	: "Areg", "Breg", "Creg", "FAreg", "FBreg", "FCreg", "Wreg[0]");
	asm __volatile
	("in"
	: /* no outputs */
	: "cP" (&version),
	  "bP" (Link0Input),
	  "aP" (4)
	: "FAreg", "FBreg", "FCreg", "memory");
	return version;
}

void exitServer(void) {
	asm __volatile
	("outword\n\t"
	: /* no outputs */
	: "bP" (Link0Output),
	  "aP" (SERVER_EXIT)
	: "Areg", "Breg", "Creg", "FAreg", "FBreg", "FCreg", "Wreg[0]");
}

void putConsoleChar(char b) {
	asm __volatile
	("outword\n\t"
	: /* no outputs */
	: "bP" (Link0Output),
	  "aP" (CONSOLE_PUT_CHAR)
	: "Areg", "Breg", "Creg", "FAreg", "FBreg", "FCreg", "Wreg[0]");
	asm __volatile
	("outbyte\n\t"
	: /* no outputs */
	: "bP" (Link0Output),
	  "aP" (b)
	: "Areg", "Breg", "Creg", "FAreg", "FBreg", "FCreg", "Wreg[0]");
}

void putConsolePString(BYTE len, char *data) {
	asm __volatile
	("outword\n\t"
	: /* no outputs */
	: "bP" (Link0Output),
	  "aP" (CONSOLE_PUT_PSTR)
	: "Areg", "Breg", "Creg", "FAreg", "FBreg", "FCreg", "Wreg[0]");
	asm __volatile
	("outbyte\n\t"
	: /* no outputs */
	: "bP" (Link0Output),
	  "aP" (len)
	: "Areg", "Breg", "Creg", "FAreg", "FBreg", "FCreg", "Wreg[0]");
	asm __volatile
	("out"
	: /* no outputs */
	: "cP" (data),
	  "bP" (Link0Output),
	  "aP" (len)
	: "Areg", "Breg", "Creg", "FAreg", "FBreg", "FCreg", "Wreg[0]");
}

static int localStrlen(char *msg) {
	int len = 0;
	while (*msg != 0) {
		len++;
		msg++;
	}
	return len;
}


void putConsoleCString(char *data) {
	BYTE null = 0;
	WORD32 len = localStrlen(data);
	asm __volatile
	("outword\n\t"
	: /* no outputs */
	: "bP" (Link0Output),
	  "aP" (CONSOLE_PUT_CSTR)
	: "Areg", "Breg", "Creg", "FAreg", "FBreg", "FCreg", "Wreg[0]");
	asm __volatile
	("out\n\t"
	: /* no outputs */
	: "cP" (data),
	  "bP" (Link0Output),
	  "aP" (len)
	: "Areg", "Breg", "Creg", "FAreg", "FBreg", "FCreg", "Wreg[0]");
	asm __volatile
	("outbyte"
	: /* no outputs */
	: "bP" (Link0Output),
	  "aP" (null)
	: "Areg", "Breg", "Creg", "FAreg", "FBreg", "FCreg", "Wreg[0]");
}

BYTE isConsolePutAvailable(void) {
	BYTE avail=0;
	asm __volatile
	("outword\n\t"
	: /* no outputs */
	: "bP" (Link0Output),
	  "aP" (CONSOLE_PUT_AVAILABLE)
	: "Areg", "Breg", "Creg", "FAreg", "FBreg", "FCreg", "Wreg[0]");
	asm __volatile
	("in"
	: /* no outputs */
	: "cP" (&avail),
	  "bP" (Link0Input),
	  "aP" (1)
	: "FAreg", "FBreg", "FCreg", "memory");
	return (avail == 1) ? 1 : 0;
}

BYTE isConsoleGetAvailable(void) {
	BYTE avail=0;
	asm __volatile
	("outword\n\t"
	: /* no outputs */
	: "bP" (Link0Output),
	  "aP" (CONSOLE_GET_AVAILABLE)
	: "Areg", "Breg", "Creg", "FAreg", "FBreg", "FCreg", "Wreg[0]");
	asm __volatile
	("in"
	: /* no outputs */
	: "cP" (&avail),
	  "bP" (Link0Input),
	  "aP" (1)
	: "FAreg", "FBreg", "FCreg", "memory");
	return (avail == 1) ? 1 : 0;
}

char getConsoleChar(void) {
	char ch=0;
	asm __volatile
	("outword\n\t"
	: /* no outputs */
	: "bP" (Link0Output),
	  "aP" (CONSOLE_GET_CHAR)
	: "Areg", "Breg", "Creg", "FAreg", "FBreg", "FCreg", "Wreg[0]");
	asm __volatile
	("in"
	: /* no outputs */
	: "cP" (&ch),
	  "bP" (Link0Input),
	  "aP" (1)
	: "FAreg", "FBreg", "FCreg", "memory");
	return ch;
}

WORD32 getTimeMillis(void) {
	WORD32 t=0;
	asm __volatile
	("outword\n\t"
	: /* no outputs */
	: "bP" (Link0Output),
	  "aP" (TIME_GET_MILLIS)
	: "Areg", "Breg", "Creg", "FAreg", "FBreg", "FCreg", "Wreg[0]");
	asm __volatile
	("in"
	: /* no outputs */
	: "cP" (&t),
	  "bP" (Link0Input),
	  "aP" (4)
	: "FAreg", "FBreg", "FCreg", "memory");
	return t;
}

void getTimeUTC(struct timeGetUTC *fill) {
	asm __volatile
	("outword\n\t"
	: /* no outputs */
	: "bP" (Link0Output),
	  "aP" (TIME_GET_UTC)
	: "Areg", "Breg", "Creg", "FAreg", "FBreg", "FCreg", "Wreg[0]");
	asm __volatile
	("in"
	: /* no outputs */
	: "cP" (fill),
	  "bP" (Link0Input),
	  "aP" (28)
	: "FAreg", "FBreg", "FCreg", "memory");
}


//------------------------------------------------------------------------------
// Private functions
//------------------------------------------------------------------------------


