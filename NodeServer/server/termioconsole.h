//------------------------------------------------------------------------------
//
// File        : termioconsole.h
// Description : Definition of termio Console.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 05/03/2019
//
// (C) 2005-2019 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _TERMIOCONSOLE_H
#define _TERMIOCONSOLE_H

#include <exception>
using namespace std;
#include <cerrno>
#include <termios.h>
#include <unistd.h>

#include "types.h"
#include "console.h"

class TermioConsole : public Console {
public:
    TermioConsole();
    void initialise(void) throw (exception);
    ~TermioConsole(void);

    bool isCharAvailable();
    BYTE getChar();
private:
    // For console keyboard handling
    int stdinfd;
    struct timeval timeout;
    fd_set stdinfdset;
    termios term, origterm;
};

#endif // _TERMIOCONSOLE_H

