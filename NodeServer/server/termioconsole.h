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

#include "types.h"
#include "console.h"

class TermioConsole : public Console {
public:
    TermioConsole();
    void initialise(void) throw (exception);
    ~TermioConsole(void);
private:
};

#endif // _TERMIOCONSOLE_H

