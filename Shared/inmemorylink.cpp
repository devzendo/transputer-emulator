//------------------------------------------------------------------------------
//
// File        : inmemorylink.cpp
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

#include "inmemorylink.h"
#include "log.h"
#include "sync.h"

//------------------------------------------------------------------------------

class ByteRegister {
public:
    ByteRegister() : m_register(0), m_storing(false) {
    }

    // Precondition: m_storing == false
    void store(BYTE8 buf) {
        MUTEX
        m_register = buf;
        m_storing = true;
    }

    // Precondition: m_storing == true
    BYTE8 read() {
        MUTEX
        m_storing = false;
        return m_register;
    }

    void read_maybe(BYTE8 *buf, bool *done) {
        MUTEX
        if (m_storing) {
            *done = true;
            *buf = m_register;
            m_storing = false;
        }
    }

    void write_maybe(const BYTE8 *buf, bool *done) {
        MUTEX
        if (!m_storing) {
            *done = true;
            m_register = *buf;
            m_storing = true;
        }
    }

    bool storing() {
        MUTEX
        return m_storing;
    }

#ifdef DESKTOP
    std::mutex m_mutex;
#endif
#ifdef PICO
    CriticalSection m_criticalsection;
#endif
    BYTE8 m_register;
    bool m_storing;
};

//------------------------------------------------------------------------------

InMemoryLink::InMemoryLink(int linkNo, void *readState, void *writeState) : Link(linkNo, false),
    myWriteSequence(0), myReadSequence(0) {
    // Although these links are used in the EmuServer between IServer and Emulator, we
    // don't need to distinguish between 'server' and 'client', as the ends are given
    // by the state pairs.
    logDebugF("Constructing InMemory link %d", myLinkNo);
    m_read_state = readState;
    m_write_state = writeState;
}

void InMemoryLink::initialise() {
    myWriteSequence = myReadSequence = 0;
}

InMemoryLink::~InMemoryLink() {
    logDebugF("Destroying InMemory link %d", myLinkNo);
}

BYTE8 InMemoryLink::readByte() {
	BYTE8 buf;
    bool done = false;
    for (;;) {
        ByteRegister *read_reg = static_cast<ByteRegister *>(m_read_state);
        read_reg->read_maybe(&buf, &done);
        if (done) {
            break;
        }
    }
    if (bDebug) {
        logDebugF("Link %d R #%08X %02X (%c)", myLinkNo, myReadSequence++, buf, isprint(buf) ? buf : '.');
    }
    return buf;
}

void InMemoryLink::writeByte(BYTE8 buf) {
    BYTE8 bufstore = buf;
    bool done = false;
    for (;;) {
        ByteRegister *write_reg = static_cast<ByteRegister *>(m_write_state);
        write_reg->write_maybe(&bufstore, &done);
        if (done) {
            break;
        }
    }
    if (bDebug) {
        logDebugF("Link %d W #%08X %02X (%c)", myLinkNo, myWriteSequence++, buf, isprint(buf) ? buf : '.');
    }
}

void InMemoryLink::resetLink() {
    // TODO
}

int InMemoryLink::getLinkType() {
    return LinkType_InMemory;
}

// Testing methods
bool InMemoryLink::_readAvailable() const {
    ByteRegister *read_reg = static_cast<ByteRegister *>(m_read_state);
    return read_reg->storing();
}

bool InMemoryLink::_writeAvailable() const {
    ByteRegister *write_reg = static_cast<ByteRegister *>(m_write_state);
    return ! write_reg->storing();
}


//------------------------------------------------------------------------------

InMemoryLinkFactory::InMemoryLinkFactory(int linkANo, int linkBNo) {
    m_state_a = new ByteRegister();
    m_state_b = new ByteRegister();
    m_linkA = new InMemoryLink(linkANo, m_state_b, m_state_a);
    m_linkB = new InMemoryLink(linkBNo, m_state_a, m_state_b);
}

Link *InMemoryLinkFactory::linkA() const {
    return reinterpret_cast<Link *>(m_linkA);
};

Link *InMemoryLinkFactory::linkB() const {
    return reinterpret_cast<Link *>(m_linkB);
};
