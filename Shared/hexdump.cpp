//------------------------------------------------------------------------------
//
// File        : hexdump.cpp
// Description : Hex/ASCII dump routines
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 19/08/2005
//
// (C) 2005-2019 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <cctype>
#include <cstdio>

#include "types.h"
#include "log.h"

static char hexdigs[]="0123456789abcdef";

void hexdump(BYTE8 *buf, WORD32 len)
{
const int lineLen = 80;
char line[lineLen];
WORD32 i;
WORD32 offset=0;
WORD32 left=len;
WORD32 upto16, x;
BYTE8 b;
	while (left > 0) {
		for (i = 0; i < 78; i++) {
			line[i] = ' ';
		}
		line[9] = line[59] = '|';
		line[77] = '\0';
		snprintf(line, lineLen, "%08X", offset);
		line[8] = ' ';
		upto16 = (left > 16) ? 16 : left;
		for (x = 0; x < upto16; x++) {
			b = buf[offset + x];
			line[11 + (3 * x)] = hexdigs[(b & 0xf0) >> 4];
			line[12 + (3 * x)] = hexdigs[b & 0x0f];
			line[61 + x] = isprint((char)b) ? ((char)b) : '.';
		}
		logDebug(line);
		offset += upto16;
		left -= upto16;
	}
}

