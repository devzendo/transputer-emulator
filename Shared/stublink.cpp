//------------------------------------------------------------------------------
//
// File        : stublink.cpp
// Description : A Link that can be mocked up with data
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 14/08/2019
//
// (C) 2005-2026 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <exception>
#include <cctype>

#include "stublink.h"
#include "log.h"

StubLink::StubLink(int linkNo, bool isServer) : SystemLink(linkNo, isServer),
                                                // Now set up the references to the queues, using those that will be used appropriately
                                                // for read/write given that the user is a server/CPU client.
                                                rdq(isServer ? &myWriteQueue : &myReadQueue), wrq(isServer ? &myReadQueue : &myWriteQueue),
                                                mWriteQueueSize(256) {
    logDebugF("Constructing stub link %d for %s", myLinkNo, isServer ? "server" : "cpu client");
}

void StubLink::initialise() {
    (void) myReadQueue.empty();
    (void) myWriteQueue.empty();
    myWriteSequence = myReadSequence = 0;
    mWriteFlushCalled = false;
}

void StubLink::setWriteQueueSize(int size) {
    mWriteQueueSize = size;
}

StubLink::~StubLink() {
    logDebugF("Destroying stub link %d", myLinkNo);
    (void) myReadQueue.empty();
    (void) myWriteQueue.empty();
}

BYTE8 StubLink::readByte() {
    BYTE8 buf = rdq->front();
    rdq->pop();
    if (bDebug) {
        logDebugF("Link %d R #%08X %02X (%c)", myLinkNo, myReadSequence++, buf, isprint(buf) ? buf : '.');
    }
    return buf;
}

void StubLink::writeByte(BYTE8 buf) {
    if (bDebug) {
        logDebugF("Link %d W #%08X %02X (%c)", myLinkNo, myWriteSequence++, buf, isprint(buf) ? buf : '.');
    }
    wrq->push(buf);
}

void StubLink::resetLink() {
    // TODO
}

std::vector<BYTE8> StubLink::getWrittenBytes() const {
    std::vector<BYTE8> out;
    while (!wrq->empty()) {
        BYTE8 data = wrq->front();
        wrq->pop();
        out.push_back(data);
    }
    return out;
}

BYTE8 StubLink::getFirstWrittenByte() const {
    if (wrq->empty()) {
        logWarn("You can't get the first written byte of an empty stublink write queue");
        return 0x00;
    }
    BYTE8 data = wrq->front();
    wrq->pop();
    return data;
}

void StubLink::setReadableBytes(const std::vector<BYTE8>& bytes) const {
    for (const BYTE8 & byte : bytes) {
        rdq->push(byte);
    }
}

int StubLink::getLinkType() {
    return LinkType_Stub;
}

bool StubLink::readAvailable() {
    return !rdq->empty();
}

bool StubLink::writeAvailable() {
    return wrq->size() < mWriteQueueSize;
}

void StubLink::writeFlush() {
    mWriteFlushCalled = true;
}

bool StubLink::writeFlushCalled() {
    return mWriteFlushCalled;
}