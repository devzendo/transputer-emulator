//------------------------------------------------------------------------------
//
// File        : console.h
// Description : Abstract base class for Console
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 05/03/2019
//
// (C) 2005-2019 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _CONSOLE_H
#define _CONSOLE_H

#include <exception>
using namespace std;

#include "platform.h"
#include "types.h"

#ifdef PLATFORM_WINDOWS
// For MSVC, shut up about throw (exception)
#pragma warning( disable : 4290 )
#endif

class Console {
public:
    Console();
    virtual void initialise(void) throw (exception);
    virtual ~Console(void);
    void setDebug(bool newDebug);
    virtual bool isCharAvailable();
    virtual BYTE getChar();
protected:
    bool bDebug;
};

#endif // _LINK_H

