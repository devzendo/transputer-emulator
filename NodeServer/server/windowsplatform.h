//------------------------------------------------------------------------------
//
// File        : windowsplatform.h
// Description : Definition of Windows Console/Timer.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 05/03/2019
//
// (C) 2005-2019 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _WINDOWSPLATFORM_H
#define _WINDOWSPLATFORM_H

#include <exception>

#include "types.h"
#include "platform.h"

class WindowsPlatform : public Platform {
public:
    WindowsPlatform();
    void initialise(void) throw (exception);
    ~WindowsPlatform(void);

    bool isConsoleCharAvailable();
    BYTE8 getConsoleChar();
    void putConsoleChar(BYTE8 const ch);

    WORD32 getTimeMillis();
    UTCTime getUTCTime();
private:
};

#endif // _WINDOWSPLATFORM_H

