//------------------------------------------------------------------------------
//
// File        : picousbseriallink.cpp
// Description : A Link over the Pi Pico USB Serial connection.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 07/10/2023
//
// (C) 2005-2024 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <exception>

#include "picousbseriallink.h"
#include "log.h"

PicoUSBSerialLink::PicoUSBSerialLink(int linkNo, bool isServer) :
    Link(linkNo, isServer) {
    logDebugF("Constructing Pico USB Serial link %d for %s", myLinkNo, isServer ? "server" : "cpu client");
}

void PicoUSBSerialLink::initialise(void) throw (std::exception) {
    myWriteSequence = myReadSequence = 0;
}

PicoUSBSerialLink::~PicoUSBSerialLink() {
    logDebugF("Destroying Pico USB Serial link %d", myLinkNo);
}

BYTE8 PicoUSBSerialLink::readByte() throw (std::exception) {
    if (bDebug) {
        logDebugF("Link %d R #%08X 00 (.)", myLinkNo, myReadSequence++);
    }
    return 0;
}

void PicoUSBSerialLink::writeByte(BYTE8 buf) throw (std::exception) {
    if (bDebug) {
        logDebugF("Link %d W #%08X 00 (.)", myLinkNo, myWriteSequence++);
    }
}

void PicoUSBSerialLink::resetLink(void) throw (std::exception) {
    // TODO
}

int PicoUSBSerialLink::getLinkType() {
    return LinkType_FIFO;
}
