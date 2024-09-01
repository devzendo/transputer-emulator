//------------------------------------------------------------------------------
//
// File        : stublink.h
// Description : A Link that can be mocked up with data
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 14/08/2019
//
// (C) 2005-2024 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _STUBLINK_H
#define _STUBLINK_H

#include <queue>
#include <vector>

#include "types.h"
#include "link.h"

class StubLink : public Link {
public:
    StubLink(int linkNo, bool isServer);
    void initialise(void);
    ~StubLink(void);
    BYTE8 readByte(void);
    void writeByte(BYTE8 b);
    void resetLink(void);
    // Used by tests to sense what has been written (by SUT calling writeByte) and
    // to inject data to be read (by SUT calling readbyte).
    std::vector<BYTE8> getWrittenBytes();
    void setReadableBytes(std::vector<BYTE8> bytes);
    int getLinkType(void);
private:
    // These are relative to the CPU, so it reads from the read queue.
    // The IServer reads from the write queue.
    std::queue<BYTE8> myReadQueue;
    std::queue<BYTE8> myWriteQueue;
    // These are set 'the right way round' for the CPU, and crosswired
    // for the IServer.
    std::queue<BYTE8> * rdq;
    std::queue<BYTE8> * wrq;
    WORD32 myWriteSequence, myReadSequence;
};

#endif // _STUBLINK_H
