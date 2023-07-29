//------------------------------------------------------------------------------
//
// File        : posixplatform.h
// Description : Definition of POSIX Console/Timer.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 05/03/2019
//
// (C) 2005-2023 Matt J. Gumbley
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

class POSIXPlatform final : public Platform {
public:
    POSIXPlatform();
    void initialise() noexcept(false) override;
    ~POSIXPlatform() override;

    bool isConsoleCharAvailable() override;
    BYTE8 getConsoleChar() override;
    void putConsoleChar(BYTE8 ch) override;

    WORD32 getTimeMillis() override;
    UTCTime getUTCTime() override;
private:
    // For console keyboard handling
    int stdinfd;
    struct timeval timeout;
    termios term, origterm;
};

#endif // _POSIXPLATFORM_H

