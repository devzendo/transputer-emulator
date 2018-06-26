//------------------------------------------------------------------------------
//
// File        : link.cpp
// Description : Abstract base class for links
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 18/07/2005
// Revision    : $Revision $
//
// (C) 2005-2018 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <exception>
using namespace std;

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

void Link::initialise(void) throw (exception) {
}

Link::~Link(void) {
	logDebugF("Destroying link %d", myLinkNo);
}

BYTE Link::readByte(void) throw (exception) {
	logError("Link::readByte called - should always be a subclass method");
	return 0x00;
}
void Link::writeByte(BYTE buf) throw (exception) {
	logError("Link::writeByte called - should always be a subclass method");
}
WORD32 Link::readWord(void) throw (exception) {
	// Input a little-endian word, LSB first MSB last
	return (readByte()) |
		  (readByte() << 8) |
		  (readByte() << 16) |
		  (readByte() << 24);
}
void Link::writeWord(WORD32 w) throw (exception) {
	// Always output as a little-endian word, LSB first MSB last
	writeByte(w & 0x000000ff);
	writeByte((w & 0x0000ff00) >> 8);
	writeByte((w & 0x00ff0000) >> 16);
	writeByte((w & 0xff000000) >> 24);
}

void Link::resetLink(void) throw (exception) {
}

int Link::getLinkNo(void) {
	return myLinkNo;
}
void Link::setDebug(bool newDebug) {
	bDebug = newDebug;
}
