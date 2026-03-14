//------------------------------------------------------------------------------
//
// File        : ttylink.cpp
// Description : Implementation of a Link that uses a TTY or COM port.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 14/03/2026
//
// (C) 2005-2026 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <string>
#include <cerrno>
#include <unistd.h>
#include <sys/fcntl.h>

#include "types.h"
#include "link.h"
#include "ttylink.h"
#include "log.h"

TTYLink::TTYLink(int linkNo, bool isServer, const std::string &ttyFileName) :
    Link(linkNo, isServer) {
    logDebugF("Constructing TTY link %d for %s", myLinkNo, isServer ? "server" : "cpu client");
    myFD = -1;
    myWriteSequence = myReadSequence = 0;
    myTTYName = ttyFileName;
    myMsgbuf[0] = 0;
}

void TTYLink::initialise() {
    myWriteSequence = myReadSequence = 0;
    logDebugF("Opening %s read/write", myTTYName.c_str());
    myFD = open(myTTYName.c_str(), O_RDWR);
    if (myFD == -1) {
        snprintf(myMsgbuf, TTY_MSGBUF_SIZE, "Could not open TTY %s: %s", myTTYName.c_str(), strerror(errno));
        throw std::runtime_error(myMsgbuf);
    }
}

TTYLink::~TTYLink() {
    logDebugF("Destroying TTY link %d", myLinkNo);
    if (myFD != -1) {
        close(myFD);
        myFD = -1;
    }
}

BYTE8 TTYLink::readByte() {
    BYTE8 buf;
    ssize_t readlen = 0;
    readlen = read(myFD, &buf, 1);
    if (readlen == 1) {
        if (bDebug) {
            logDebugF("Link %d R #%08X %02X (%c)", myLinkNo, myReadSequence++, buf, isprint(buf) ? buf : '.');
        }
        return buf;
    }
    snprintf(myMsgbuf, TTY_MSGBUF_SIZE,"Could not read a byte from TTY FD#%d: (read %ld byte(s)) %s", myFD, readlen, strerror(errno));
    logWarn(myMsgbuf);
    throw std::runtime_error(myMsgbuf);
}

void TTYLink::writeByte(BYTE8 buf) {
    BYTE8 bufstore = buf;
    if (bDebug) {
        logDebugF("Link %d W #%08X %02X (%c)", myLinkNo, myWriteSequence++, buf, isprint(buf) ? buf : '.');
    }
    if (write(myFD, &bufstore, 1) == 1) {
        return;
    }
    snprintf(myMsgbuf, TTY_MSGBUF_SIZE, "Could not write a byte to TTY FD#%d: %s", myFD, strerror(errno));
    throw std::runtime_error(myMsgbuf);
}

void TTYLink::resetLink() {
    // TODO
}

int TTYLink::getLinkType() {
    return LinkType_TTY;
}
