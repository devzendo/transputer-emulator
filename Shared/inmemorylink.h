//------------------------------------------------------------------------------
//
// File        : inmemorylink.h
// Description : A thread-safe link with two ends, between EmuServer and Emulator
//               or between two Emulator instances.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 07/07/2026
//
// (C) 2005-2026 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef INMEMORYLINK_H
#define INMEMORYLINK_H

#include "types.h"
#include "link.h"

class InMemoryLink : public Link {
public:
    InMemoryLink(int linkNo, bool isServer);
    void initialise(void);
    ~InMemoryLink(void);
    BYTE8 readByte(void);
    void writeByte(BYTE8 b);
    void resetLink(void);
    int getLinkType(void);
private:
    WORD32 myWriteSequence, myReadSequence;
};

#endif // INMEMORYLINK_H
