//------------------------------------------------------------------------------
//
// File        : consolefactory.h
// Description : Factory for creating derived classes of Console
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 05/03/2019
//
// (C) 2005-2019 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _CONSOLEFACTORY_H
#define _CONSOLEFACTORY_H

class ConsoleFactory {
public:
    ConsoleFactory(bool isDebug);
    Console *createConsole();
private:
    bool bDebug;
};

#endif // _CONSOLEFACTORY_H

