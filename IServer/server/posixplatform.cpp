//------------------------------------------------------------------------------
//
// File        : posixplatform.cpp
// Description : The POSIX (OSX/Linux) Console/Timer
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 05/03/2019
//
// (C) 2005-2020 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <exception>
#include <stdexcept>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#include "types.h"
#include "constants.h"
#include "platform.h"
#include "posixplatform.h"
#include "log.h"

POSIXPlatform::POSIXPlatform() : Platform() {
    logDebug("Constructing POSIX platform");
    stdinfd = 0;
}

void POSIXPlatform::initialise() noexcept(false) {
    static char msgbuf[255];

    logDebug("Initialising POSIX platform");
    // initialise console keyboard handling
    stdinfd = fileno(stdin); // should be 0!
    timeout.tv_sec = 0; // cause select to return immediately
    timeout.tv_usec = 0;
    // TODO initialise cooked input so it returns without waiting for EOL
    // initialise links
    if (isatty(stdinfd)) { // It's not a TTY in tests, where the following tcget/setattr fails.
        logDebugF("Setting stdin (fd %d) terminal attributes as it is a TTY", stdinfd);
        if (tcgetattr(stdinfd, &term) == -1) {
            sprintf(msgbuf, "Could not get stdin (fd %d) terminal attributes: %s",
                      stdinfd, strerror(errno));
            logFatal(msgbuf);
            throw std::runtime_error(msgbuf);
        }
        tcgetattr(stdinfd, &origterm);
        term.c_lflag = term.c_lflag & (~ICANON);
        if (tcsetattr(stdinfd, TCSANOW, &term) == -1) {
            sprintf(msgbuf, "Could not set stdin (fd %d) terminal attributes: %s",
                      stdinfd, strerror(errno));
            logFatal(msgbuf);
            throw std::runtime_error(msgbuf);
        }
    } else {
        logDebugF("Not setting stdin (fd %d) terminal attributes as it is not a TTY", stdinfd);
    }
}

POSIXPlatform::~POSIXPlatform() {
    logDebug("Destroying POSIX platform");
    if (isatty(stdinfd)) {
        logDebugF("Resetting stdin (fd %d) terminal attributes as it is a TTY", stdinfd);
        tcsetattr(stdinfd, TCSANOW, &origterm);
    } else {
        logDebugF("Not resetting stdin (fd %d) terminal attributes as it is not a TTY", stdinfd);
    }
}

bool POSIXPlatform::isConsoleCharAvailable() {
    BYTE8 ready = 0;
    fd_set stdinfdset;
    for (;;) {
        FD_ZERO(&stdinfdset);
        FD_SET(stdinfd, &stdinfdset);
        int fds = select(stdinfd+1, &stdinfdset, NULL, NULL, &timeout);
        if (fds == -1) {
            if (errno == EINTR) {
                continue; // try again
            }
            logWarnF("isConsoleCharAvailable select failed: %s", strerror(errno));
            break;
        }
        if (fds == 0) {
            break;
        }
        if (fds == 1) {
            ready = 1;
            break;
        }
        logWarnF("isConsoleCharAvailable select returned %d", fds);
        break;
    }
    return ready;
}

BYTE8 POSIXPlatform::getConsoleChar() {
    BYTE8 inChar;
    read(stdinfd, &inChar, 1);
    return inChar;
}

void POSIXPlatform::putConsoleChar(const BYTE8 ch) {
    // TODO might be better to setvbuf on stdout, and undo this on terminate. Write there?
    fputc(ch, stderr);
}


WORD32 POSIXPlatform::getTimeMillis() {
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    return (tv.tv_sec*1000) + (tv.tv_usec/1000); // TODO check this transform
}

UTCTime POSIXPlatform::getUTCTime() {
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    struct tm *tms = gmtime(&tv.tv_sec);
    return UTCTime(tms->tm_mday, tms->tm_mon + 1, tms->tm_year + 1900, tms->tm_hour, tms->tm_min, tms->tm_sec, (tv.tv_usec/1000));
}
