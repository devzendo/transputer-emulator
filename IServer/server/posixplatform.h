//------------------------------------------------------------------------------
//
// File        : posixplatform.h
// Description : Definition of POSIX Console/Timer.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 05/03/2019
//
// (C) 2005-2019 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _POSIXPLATFORM_H
#define _POSIXPLATFORM_H

#include <exception>

#include <cerrno>
#include <termios.h>
#include <unistd.h>
#include <sys/time.h>

#include "types.h"
#include "platform.h"

class POSIXPlatform : public Platform {
public:
    POSIXPlatform();
    void initialise(void) throw (std::exception);
    ~POSIXPlatform(void);

    bool isConsoleCharAvailable();
    BYTE8 getConsoleChar();
    void putConsoleChar(const BYTE8 ch);

    WORD32 getTimeMillis();
    UTCTime getUTCTime();
private:
    // For console keyboard handling
    int stdinfd;
    struct timeval timeout;
    fd_set stdinfdset;
    termios term, origterm;
};

#endif // _POSIXPLATFORM_H
