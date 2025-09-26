//------------------------------------------------------------------------------
//
// File        : nulllink.cpp
// Description : A Link that can just sends nulls
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 13/09/2023
//
// (C) 2005-2024 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <cctype>

#include "nulllink.h"
#include "log.h"

NullLink::NullLink(int linkNo, bool isServer) :
    Link(linkNo, isServer) {
    logDebugF("Constructing Null link %d for %s", myLinkNo, isServer ? "server" : "cpu client");
}

void NullLink::initialise(void) {
    myWriteSequence = myReadSequence = 0;
}

NullLink::~NullLink() {
    logDebugF("Destroying Null link %d", myLinkNo);
}

BYTE8 NullLink::readByte() {
    if (bDebug) {
        logDebugF("Link %d R #%08X 00 (.)", myLinkNo, myReadSequence++);
    }
    return 0;
}

void NullLink::writeByte(BYTE8 buf) {
    if (bDebug) {
        logDebugF("Link %d W #%08X 00 (.)", myLinkNo, myWriteSequence++);
    }
}

void NullLink::resetLink(void) {
    // TODO
}

int NullLink::getLinkType() {
    return LinkType_Null;
}

void NullLink::poll(void) {
	// no-op
}