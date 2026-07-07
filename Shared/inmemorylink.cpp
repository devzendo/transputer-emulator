//------------------------------------------------------------------------------
//
// File        : inmemorylink.cpp
// Description : A thread-safe link with two ends, between EmuServer and Emulator
//               or between two Emulator instances.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 07/07/2026
//
// (C) 2005-2026 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <cctype>

#include "inmemorylink.h"

#include "log.h"

InMemoryLink::InMemoryLink(int linkNo, bool isServer) :
    Link(linkNo, isServer) {
    logDebugF("Constructing InMemory link %d for %s", myLinkNo, isServer ? "server" : "cpu client");
    // TODO allow creation of the link twice, with different ends based on the isServer flag
}

void InMemoryLink::initialise(void) {
    myWriteSequence = myReadSequence = 0;
}

InMemoryLink::~InMemoryLink() {
    logDebugF("Destroying InMemory link %d", myLinkNo);
}

BYTE8 InMemoryLink::readByte() {
    if (bDebug) {
        logDebugF("Link %d R #%08X 00 (.)", myLinkNo, myReadSequence++);
    }
    return 0;
}

void InMemoryLink::writeByte(BYTE8 buf) {
    if (bDebug) {
        logDebugF("Link %d W #%08X 00 (.)", myLinkNo, myWriteSequence++);
    }
}

void InMemoryLink::resetLink(void) {
    // TODO
}

int InMemoryLink::getLinkType() {
    return LinkType_InMemory;
}
