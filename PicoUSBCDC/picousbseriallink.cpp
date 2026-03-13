//------------------------------------------------------------------------------
//
// File        : picousbseriallink.cpp
// Description : A Link over the Pi Pico USB Serial connection.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 07/10/2023
//
// (C) 2005-2026 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <exception>
#include <cctype>

#include "picousbseriallink.h"

#include "cdc_app.h"
#include "../Shared/log.h"

PicoUSBSerialLink::PicoUSBSerialLink(int linkNo, bool isServer) :
    Link(linkNo, isServer) {
    logDebugF("Constructing Pico USB Serial link %d for %s", myLinkNo, isServer ? "server" : "cpu client");
}

void PicoUSBSerialLink::initialise(void) {
    myWriteSequence = myReadSequence = 0;
}

PicoUSBSerialLink::~PicoUSBSerialLink() {
    logDebugF("Destroying Pico USB Serial link %d", myLinkNo);
}

BYTE8 PicoUSBSerialLink::readByte() {
    BYTE8 buf;
    uint32_t readlen = usb_link_read(&buf, 1);
    if (readlen == 1) {
        if (bDebug) {
            logDebugF("Link %d R #%08X %02X (%c)", myLinkNo, myReadSequence++, buf, isprint(buf) ? buf : '.');
        }
        return buf;
    }
    logWarnF("Could not read a byte from USB CDC link: (read %d byte(s))", readlen);
    return 0;
}

void PicoUSBSerialLink::writeByte(BYTE8 buf) {
    BYTE8 bufstore = buf;
    if (bDebug) {
        logDebugF("Link %d W #%08X %02X (%c)", myLinkNo, myWriteSequence++, buf, isprint(buf) ? buf : '.');
    }
    usb_link_write(&bufstore, 1);
}

void PicoUSBSerialLink::resetLink(void) {
    // TODO
}

int PicoUSBSerialLink::getLinkType() {
    return LinkType_USBCDC;
}
