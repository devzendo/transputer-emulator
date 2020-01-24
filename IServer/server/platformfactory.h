//------------------------------------------------------------------------------
//
// File        : platformfactory.h
// Description : Factory for creating derived classes of Platform
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 05/03/2019
//
// (C) 2005-2020 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _PLATFORMFACTORY_H
#define _PLATFORMFACTORY_H

class PlatformFactory {
public:
    PlatformFactory(bool isDebug);
    Platform *createPlatform();
private:
    bool bDebug;
};

#endif // _PLATFORMFACTORY_H

