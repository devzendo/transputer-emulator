//------------------------------------------------------------------------------
//
// File        : posixplatform.cpp
// Description : The POSIX (OSX/Linux) Console/Timer
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 05/03/2019
//
// (C) 2005-2019 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <exception>
#include <stdexcept>
#include <termios.h>
using namespace std;
#include "types.h"
#include "constants.h"
#include "platform.h"
#include "posixplatform.h"
#include "log.h"

POSIXPlatform::POSIXPlatform() : Platform() {
    logDebug("Constructing POSIX platform");
}

void POSIXPlatform::initialise(void) throw (exception) {
    static char msgbuf[255];

    logDebug("Initialising termio console");
    // initialise console keyboard handling
    stdinfd = fileno(stdin); // should be 0!
    timeout.tv_sec = 0; // cause select to return immediately
    timeout.tv_usec = 0;
    // TODO initialise cooked input so it returns without waiting for EOL
    // initialise links
    if (tcgetattr(stdinfd, &term) == -1) {
        sprintf(msgbuf, "Could not get stdin terminal attributes: %s",
                  strerror(errno));
        logFatal(msgbuf);
        throw runtime_error(msgbuf);
    }
    tcgetattr(stdinfd, &origterm);
    term.c_lflag = term.c_lflag & (~ICANON);
    if (tcsetattr(stdinfd, TCSANOW, &term) == -1) {
        sprintf(msgbuf, "Could not set stdin terminal attributes: %s",
                  strerror(errno));
        logFatal(msgbuf);
        throw runtime_error(msgbuf);
    }

}

POSIXPlatform::~POSIXPlatform() {
    logDebug("Destroying POSIX platform");
    tcsetattr(stdinfd, TCSANOW, &origterm);
}

bool POSIXPlatform::isCharAvailable() {
    BYTE ready = 0;
    for (;;) {
        FD_ZERO(&stdinfdset);
        FD_SET(stdinfd, &stdinfdset);
        int fds = select(stdinfd+1, &stdinfdset, NULL, NULL, &timeout);
        if (fds == -1) {
            if (errno == EINTR) {
                continue; // try again
            }
            logWarnF("isCharAvailable select failed: %s", strerror(errno));
            break;
        }
        if (fds == 0) {
            break;
        }
        if (fds == 1) {
            ready = 1;
            break;
        }
        logWarnF("isCharAvailable select returned %d", fds);
        break;
    }
    return ready;
}

BYTE POSIXPlatform::getChar() {
    BYTE inChar;
    read(stdinfd, &inChar, 1);
    return inChar;
}

void POSIXPlatform::putChar(BYTE const ch) {
    // TODO might be better to setvbuf on stdout, and undo this on terminate. Write there?
    fputc(ch, stderr);
}


WORD32 POSIXPlatform::getTimeMillis() {
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    return (tv.tv_sec*1000) + (tv.tv_usec/1000); // TODO check this transform
}