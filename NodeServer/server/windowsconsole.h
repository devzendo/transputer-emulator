//------------------------------------------------------------------------------
//
// File        : windowsconsole.h
// Description : Definition of Windows Console.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 05/03/2019
//
// (C) 2005-2019 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _WINDOWSCONSOLE_H
#define _WINDOWSCONSOLE_H

#include <exception>
using namespace std;

#include "types.h"
#include "console.h"

class WindowsConsole : public Console {
public:
    WindowsConsole();
    void initialise(void) throw (exception);
    ~WindowsConsole(void);

    bool isCharAvailable();
    BYTE getChar();
private:
};

#endif // _WINDOWSCONSOLE_H

