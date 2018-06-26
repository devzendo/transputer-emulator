//------------------------------------------------------------------------------
//
// File        : nodeclient.cpp
// Description : library of functions that encapsulate the node server
//               protocol, for client code to use.
// License     : GNU GPL - see COPYING for more details
// Created     : 06/09/2005
// Revision    : $Revision $
//
// (C) 2005 Matt J. Gumbley
// matt@gumbley.me.uk
// http://www.gumbley.me.uk/parachute
//
//------------------------------------------------------------------------------

#include "types.h"
#include "nsproto.h"
#include "nodeclient.h"

const WORD32 Link3Input=0x8000001C;
const WORD32 Link2Input=0x80000018;
const WORD32 Link1Input=0x80000014;
const WORD32 Link0Input=0x80000010;
const WORD32 Link3Output=0x8000000C;
const WORD32 Link2Output=0x80000008;
const WORD32 Link1Output=0x80000004;
const WORD32 Link0Output=0x80000000;

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

void putConsoleChar(BYTE b) {
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

void putConsolePString(BYTE len, BYTE *data) {
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

void putConsoleCString(BYTE *data) {
	BYTE null = 0;
	WORD32 len;
	register int i;
	for (i=0; i<CONSOLE_PUT_CSTR_BUF_LIMIT; i++) {
		if (data != '\0') {
			len++;
		}
	}
	asm __volatile
	("outword\n\t"
	: /* no outputs */
	: "bP" (Link0Output),
	  "aP" (CONSOLE_PUT_CSTR)
	: "Areg", "Breg", "Creg", "FAreg", "FBreg", "FCreg", "Wreg[0]");
	asm __volatile
	("out"
	: /* no outputs */
	: "cP" (data),
	  "bP" (Link0Output),
	  "aP" (len)
	: "Areg", "Breg", "Creg", "FAreg", "FBreg", "FCreg", "Wreg[0]");
	asm __volatile
	("outbyte\n\t"
	: /* no outputs */
	: "bP" (Link0Output),
	  "aP" (null)
	: "Areg", "Breg", "Creg", "FAreg", "FBreg", "FCreg", "Wreg[0]");
}

bool isConsolePutAvailable(void) {
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
	return (avail == 1);
}

bool isConsoleGetAvailable(void) {
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
	return (avail == 1);
}

BYTE getConsoleChar(void) {
	BYTE ch=0;
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


