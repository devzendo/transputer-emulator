//------------------------------------------------------------------------------
//
// File        : asynclink.cpp
// Description : An asynchronous link that works with a pair of (abstract)
//               GPIO pins.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 26/09/2025
//
// (C) 2005-2025 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <exception>

#include "asynclink.h"
#include "log.h"

AsyncLink::AsyncLink(int linkNo, bool isServer) :
    Link(linkNo, isServer) {
    logDebugF("Constructing async link %d for %s", myLinkNo, isServer ? "server" : "cpu client");
    myWriteSequence = myReadSequence = 0;
}

void AsyncLink::initialise(void) {
    
}

AsyncLink::~AsyncLink() {
    logDebugF("Destroying async link %d", myLinkNo);
}

BYTE8 AsyncLink::readByte() {
    if (bDebug) {
        logDebugF("Link %d R #%08X 00 (.)", myLinkNo, myReadSequence++);
    }
    return 0;
}

void AsyncLink::writeByte(BYTE8 buf) {
    if (bDebug) {
        logDebugF("Link %d W #%08X 00 (.)", myLinkNo, myWriteSequence++);
    }
}

void AsyncLink::resetLink(void) {
    // TODO
}

int AsyncLink::getLinkType() {
    return LinkType_Async;
}

void AsyncLink::poll(void) {
	// no-op
}