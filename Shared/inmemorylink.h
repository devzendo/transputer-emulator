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
    InMemoryLink(int linkNo, void *readState, void *writeState);
    void initialise(void);
    BYTE8 readByte(void);
    void writeByte(BYTE8 b);
    void resetLink(void);
    int getLinkType(void);
    ~InMemoryLink(void);
    // Testing methods
    bool _readAvailable() const;
    bool _writeAvailable() const;
private:
    WORD32 myWriteSequence{}, myReadSequence{};
    void *m_write_state; // an internal object
    void *m_read_state; // an internal object
};

class InMemoryLinkFactory {
public:
    // Create a connected pair of links, giving them whatever link numbers make sense for the devices they'll be
    // connected to.
    InMemoryLinkFactory(int linkANo, int linkBNo);
    Link *linkA() const;
    Link *linkB() const;
    ~InMemoryLinkFactory() = default;
private:
    void *m_state_a; // an internal object
    void *m_state_b; // an internal object
    InMemoryLink *m_linkA;
    InMemoryLink *m_linkB;
};

#endif // INMEMORYLINK_H
