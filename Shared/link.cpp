//------------------------------------------------------------------------------
//
// File        : link.cpp
// Description : Abstract base class for links
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 18/07/2005
//
// (C) 2005-2023 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <exception>

#include "types.h"
#include "constants.h"
#include "link.h"
#include "log.h"

Link::Link(int linkNo, bool isServer) {
	myLinkNo = linkNo;
	bServer = isServer;
	bDebug = false;
	logDebugF("Constructing %s link %d", bServer ? "server" : "CPU client", myLinkNo);
}

Link::~Link(void) {
	logDebugF("Destroying link %d", myLinkNo);
}

int Link::readBytes(BYTE8* buffer, int bytesToRead) throw (std::exception) {
    // TODO improve inefficiency
    int readCount = 0;
    for (; readCount < bytesToRead; readCount++) {
        buffer[readCount] = readByte();
    }
    return readCount;
}

int Link::writeBytes(BYTE8* buffer, int bytesToWrite) throw (std::exception) {
    // TODO improve inefficiency
    int writtenCount = 0;
    for (; writtenCount < bytesToWrite; writtenCount++) {
        writeByte(buffer[writtenCount]);
    }
    return writtenCount;
}

WORD16 Link::readShort(void) throw (std::exception) {
    // Input a little-endian short, LSB first MSB last
    return (readByte()) |
           (readByte() << 8);
}

void Link::writeShort(WORD16 w) throw (std::exception) {
    // Always output as a little-endian short, LSB first MSB last
    writeByte(w & 0x00ff);
    writeByte((w & 0xff00) >> 8);
}

WORD32 Link::readWord(void) throw (std::exception) {
	// Input a little-endian word, LSB first MSB last
	return (readByte()) |
		  (readByte() << 8) |
		  (readByte() << 16) |
		  (readByte() << 24);
}

void Link::writeWord(WORD32 w) throw (std::exception) {
	// Always output as a little-endian word, LSB first MSB last
	writeByte(w & 0x000000ff);
	writeByte((w & 0x0000ff00) >> 8);
	writeByte((w & 0x00ff0000) >> 16);
	writeByte((w & 0xff000000) >> 24);
}

int Link::getLinkNo(void) {
	return myLinkNo;
}

void Link::setDebug(bool newDebug) {
	bDebug = newDebug;
}
