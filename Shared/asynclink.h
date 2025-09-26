//------------------------------------------------------------------------------
//
// File        : asynclink.h
// Description : An asynchronous link that works with a pair of (abstract)
//               GPIO pins.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 26/09/2025
//
// (C) 2005-2025 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _ASYNCLINK_H
#define _ASYNCLINK_H

#include <exception>

#include "types.h"
#include "link.h"

class AsyncLink : public Link {
public:
    AsyncLink(int linkNo, bool isServer);
    void initialise(void);
    ~AsyncLink(void);
    BYTE8 readByte(void);
    void writeByte(BYTE8 b);
    void resetLink(void);
    int getLinkType(void);
    void poll(void);
private:
    WORD32 myWriteSequence, myReadSequence;
};

#endif // _ASYNCLINK_H
