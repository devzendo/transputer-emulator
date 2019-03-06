//------------------------------------------------------------------------------
//
// File        : platform.h
// Description : Abstract base class for Console, Time
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 05/03/2019
//
// (C) 2005-2019 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _PLATFORM_H
#define _PLATFORM_H

#include <exception>
using namespace std;

#include "platformdetection.h"
#include "types.h"

#ifdef PLATFORM_WINDOWS
// For MSVC, shut up about throw (exception)
#pragma warning( disable : 4290 )
#endif

class Platform {
public:
    Platform();
    virtual void initialise(void) throw (exception);
    virtual ~Platform(void);
    void setDebug(bool newDebug);
    virtual bool isCharAvailable();
    virtual BYTE getChar();
protected:
    bool bDebug;
};

#endif // _PLATFORM_H

