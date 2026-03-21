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
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
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
    struct termios serialParameters{};
    myWriteSequence = myReadSequence = 0;
    // We want to open the port in nodelay mode, so we are informed if the
    // port cannot be opened.
    logDebugF("Opening %s read/write", myTTYName.c_str());
    myFD = open(myTTYName.c_str(), O_RDWR | O_NDELAY);
	logDebugF("Opened %s read/write, fd=%d", myTTYName.c_str(), myFD);
    if (myFD == -1) {
        snprintf(myMsgbuf, TTY_MSGBUF_SIZE, "Could not open TTY %s: %s", myTTYName.c_str(), strerror(errno));
        logWarn(myMsgbuf);
        throw std::runtime_error(myMsgbuf);
    }

    // Now change back to delayed mode.
    if (fcntl(myFD, F_SETFL, fcntl (myFD, F_GETFL) & (~O_NDELAY)) == -1) {
        snprintf(myMsgbuf, TTY_MSGBUF_SIZE, "Cannot change O_NDELAY on TTY %s: %s", myTTYName.c_str(), strerror(errno));
        logWarn(myMsgbuf);
        throw std::runtime_error(myMsgbuf);
    }

    // Now change the rest of the parameters - non-canonical input, etc.
    if (tcgetattr(myFD, &serialParameters) == -1) {
        snprintf(myMsgbuf, TTY_MSGBUF_SIZE, "Cannot tcgetattr on TTY %s: %s", myTTYName.c_str(), strerror(errno));
        logWarn(myMsgbuf);
        throw std::runtime_error(myMsgbuf);
    }
//
//     (void) ioctl (fd, TCGETA, &OriginalSerialParameters);
//
    // Baud rate: https://forums.raspberrypi.com/viewtopic.php?t=351728
    serialParameters.c_cflag = B115200 | CS8 | CLOCAL | CREAD | CRTSCTS; // Fake baud rate, it's USB CDC.
    serialParameters.c_lflag = 0;
    serialParameters.c_oflag = 0;
    serialParameters.c_iflag = IGNBRK | IGNPAR;
    for (unsigned char & i : serialParameters.c_cc)
        i = static_cast<cc_t>(0);
    // http://unixwiz.net/techtips/termios-vmin-vtime.html
    serialParameters.c_cc[VMIN] = static_cast<cc_t>(0);
    serialParameters.c_cc[VTIME] = static_cast<cc_t>(20) /* DataTimeout */;

    if (tcsetattr(myFD, TCSANOW, &serialParameters) == -1) {
        snprintf(myMsgbuf, TTY_MSGBUF_SIZE, "Cannot tcsetattr on TTY %s: %s", myTTYName.c_str(), strerror(errno));
        logWarn(myMsgbuf);
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
	logDebugF("Reading byte from TTY link %d", myLinkNo);
    while (readlen == 0) {
        readlen = read(myFD, &buf, 1);
        switch (readlen) {
            case 1:
                if (bDebug) {
                    logDebugF("Link %d R #%08X %02X (%c)", myLinkNo, myReadSequence++, buf, isprint(buf) ? buf : '.');
                }
                return buf;
            case 0: // repeat
                break;
            default:
                snprintf(myMsgbuf, TTY_MSGBUF_SIZE, "Could not read a byte from TTY FD#%d: (read %ld byte(s)) %s", myFD, readlen, strerror(errno));
                logWarn(myMsgbuf);
                // throw std::runtime_error(myMsgbuf);
        }
    }
    return 0;
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
