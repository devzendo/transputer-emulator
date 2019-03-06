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
using namespace std;
#include <cerrno>
#include <termios.h>
#include <unistd.h>
#include <sys/time.h>

#include "types.h"
#include "platform.h"

class POSIXPlatform : public Platform {
public:
    POSIXPlatform();
    void initialise(void) throw (exception);
    ~POSIXPlatform(void);

    bool isCharAvailable();
    BYTE getChar();
    void putChar(BYTE const ch);

    WORD32 getTimeMillis();
private:
    // For console keyboard handling
    int stdinfd;
    struct timeval timeout;
    fd_set stdinfdset;
    termios term, origterm;
};

#endif // _POSIXPLATFORM_H

