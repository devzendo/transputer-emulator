//------------------------------------------------------------------------------
//
// File        : testlink.cpp
// Description : Tests for link abstraction
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 19/03/2019
//
// (C) 2005-2026 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <thread>
#include <atomic>

#include "inmemorylink.h"
#include "gtest/gtest.h"
#include "link.h"
#include "linkfactory.h"
#include "log.h"

// Note that links are currently blocking, and synchronous. If more than PIPE_BUF bytes are written, write() will block
// - see man 7 pipe.

using LinkPair = std::pair<Link *, Link *>;
typedef LinkPair* FactoryFunc();

class LinkPairTest : public ::testing::TestWithParam<FactoryFunc*> {
    public:
    virtual ~LinkPairTest() { delete pair;}

    void SetUp() override {
        pair = (*GetParam())();
        cpuLink = pair->first;
        serverLink = pair->second;
    }

    void TearDown() override {
        logDebug("Resetting CPU Link");
        cpuLink->resetLink();
        delete cpuLink;
        logDebug("Resetting Server Link");
        serverLink->resetLink();
        delete serverLink;

        delete pair;
        pair = nullptr;
    }

    void finished() {
        done.store(true, std::memory_order_release);
    }

    void waitForFinished() {
        while (!done.load(std::memory_order_acquire)) {
            std::this_thread::yield();
        }
        if (m_thread != nullptr) {
            logDebug("Joining thread");
            m_thread->join();
            logDebug("Thread joined");
        }
    }

    LinkPair *pair = nullptr;
    Link * cpuLink = nullptr;
    Link * serverLink = nullptr;
    std::thread *m_thread = nullptr;
    std::atomic<bool> done{false};
};

LinkPair * FactoryInMemory() {
    const auto * factory = new InMemoryLinkFactory(2, 3);

    Link * a = dynamic_cast<InMemoryLink *>(factory->linkA());
    a->setDebug(true);
    a->initialise();
    Link * b = dynamic_cast<InMemoryLink *>(factory->linkB());
    b->setDebug(true);
    b->initialise();

    return new LinkPair(a, b);
};

LinkPair * FactoryFifo() {
    auto * cpuLinkFactory = new LinkFactory(false, true);
    auto * serverLinkFactory = new LinkFactory(true, true);

    logDebug("Creating CPU Link");
    Link * cpuLink = cpuLinkFactory->createLink(0);
    cpuLink->setDebug(true);
    logDebug("Initialising CPU Link");
    cpuLink->initialise();

    logDebug("Creating Server Link");
    Link * serverLink = serverLinkFactory->createLink(0);
    serverLink->setDebug(true);
    logDebug("Initialising Server Link");
    serverLink->initialise();

    return new LinkPair(cpuLink, serverLink);
}

// What factories (types of links) can we construct here, so we have a standard set of tests for all links? Such links
// must be cross-wired. Certain link types can be, certain types definitely can't.
//
// ttylink (posix) - a pair of pty 'master/slave' (not my terminology) can be used to cross-wire two of these, but could
// be tricky to construct, requiring the pair of 'master' halves to poll each other... this'd need a separate thread,
// doing the polling. The factory could have a start/stop method that handles the thread; the factory would need to be
// stored in LinkPairTest, with start/stop called from SetUp/TearDown. Or, could use socat?
//
// commlink (windows) - similarly, can a virtual com port be instantiated? virtual serial cable; too tricky to do.
//
// gpioasynclink - Link and AsyncLink isn't merged yet - TODO
// Note TestInMemoryLink has several availability tests that can be moved here once they're merged.
//
// picousbseriallink - hardware based; would require a real Pico connected, with a ttylink on the test runner side.
// nulllink - is single-ended, so there's no other end to sense/control.
// stublink, tvslink - can't be tested like this, it can't be cross-wired; one half is connected to the CPU, the other
// does I/O for a test (stublink) or sends a program (tvslink).

INSTANTIATE_TEST_CASE_P(
    ParameterisedLinkPairTest,
    LinkPairTest,
    testing::Values(&FactoryFifo, &FactoryInMemory));

TEST_P(LinkPairTest, CPUWriteAndReadByte) {
    cpuLink->writeByte(16);
    EXPECT_EQ(serverLink->readByte(), 16);
}

TEST_P(LinkPairTest, CPUWriteAndReadBytes) {
    m_thread = new std::thread([this] {
        BYTE8 writeBuf[4] = { 0xff, 0x7f, 0x60, 0x21 };
        int bytesWritten = cpuLink->writeBytes(writeBuf, 4);
        EXPECT_EQ(bytesWritten, 4);
        finished();
    });

    BYTE8 readBuf[4];
    int bytesRead = serverLink->readBytes(readBuf, 4);
    EXPECT_EQ(bytesRead, 4);

    EXPECT_EQ(readBuf[0], 0xff);
    EXPECT_EQ(readBuf[1], 0x7f);
    EXPECT_EQ(readBuf[2], 0x60);
    EXPECT_EQ(readBuf[3], 0x21);
    waitForFinished();
}

TEST_P(LinkPairTest, CPUWriteAndReadShort) {
    m_thread = new std::thread([this] {
        cpuLink->writeShort(0x0102);
        finished();
    });

    EXPECT_EQ(serverLink->readShort(), 0x0102);
    waitForFinished();
}

TEST_P(LinkPairTest, CPUWriteAndReadWord) {
    m_thread = new std::thread([this] {
        cpuLink->writeWord(0x01020304);
        finished();
    });

    EXPECT_EQ(serverLink->readWord(), 0x01020304);
    waitForFinished();
}

TEST_P(LinkPairTest, ServerWriteAndReadByte) {
    m_thread = new std::thread([this] {
        serverLink->writeByte(32);
        finished();
    });

    EXPECT_EQ(cpuLink->readByte(), 32);
    waitForFinished();
}

TEST_P(LinkPairTest, ServerWriteAndReadWord) {
    m_thread = new std::thread([this] {
        serverLink->writeWord(0x05060708);
        finished();
    });

    EXPECT_EQ(cpuLink->readWord(), 0x05060708);
    waitForFinished();
}
