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

class LinkTest : public ::testing::Test {
protected:

    void SetUp() override {
        setLogLevel(LOGLEVEL_DEBUG);
        logDebug("SetUp start");

        cpuLinkFactory = new LinkFactory(false, true);
        serverLinkFactory = new LinkFactory(true, true);

        logDebug("Creating CPU Link");
        cpuLink = cpuLinkFactory->createLink(0);
        cpuLink->setDebug(true);
        logDebug("Initialising CPU Link");
        cpuLink->initialise();

        logDebug("Creating Server Link");
        serverLink = serverLinkFactory->createLink(0);
        serverLink->setDebug(true);
        logDebug("Initialising Server Link");
        serverLink->initialise();

        logDebug("Setup complete");
        logFlush();
    }

    void TearDown() override {
        logDebug("TearDown start");
        if (cpuLink != nullptr) {
            logDebug("Resetting CPU Link");
            cpuLink->resetLink();
	        delete cpuLink;
        }
        if (serverLink != nullptr) {
            logDebug("Resetting Server Link");
            serverLink->resetLink();
	        delete serverLink;
        }
        logDebug("TearDown complete");
        logFlush();
    }

    LinkFactory *cpuLinkFactory = nullptr;
    LinkFactory *serverLinkFactory = nullptr;
    Link *cpuLink = nullptr;
    Link *serverLink = nullptr;
};

// Note that links are currently blocking, and synchronous. If more than PIPE_BUF bytes are written, write() will block
// - see man 7 pipe.

TEST_F(LinkTest, CPUWriteAndReadByte) {
    cpuLink->writeByte(16);
    EXPECT_EQ(serverLink->readByte(), 16);
}

TEST_F(LinkTest, CPUWriteAndReadBytes) {
    BYTE8 writeBuf[4] = { 0xff, 0x7f, 0x60, 0x21 };
    int bytesWritten = cpuLink->writeBytes(writeBuf, 4);
    EXPECT_EQ(bytesWritten, 4);

    BYTE8 readBuf[4];
    int bytesRead = serverLink->readBytes(readBuf, 4);
    EXPECT_EQ(bytesRead, 4);

    EXPECT_EQ(readBuf[0], 0xff);
    EXPECT_EQ(readBuf[1], 0x7f);
    EXPECT_EQ(readBuf[2], 0x60);
    EXPECT_EQ(readBuf[3], 0x21);
}

// Server named pipe on windows blocks on ConnectNamedPipe. Need better mechanism.
//TEST_F(LinkTest, ServerWriteAndReadByte) {
//    serverLink->writeByte(32);
//    EXPECT_EQ(cpuLink->readByte(), 32);
//}

TEST_F(LinkTest, CPUWriteAndReadShort) {
    cpuLink->writeShort(0x0102);
    EXPECT_EQ(serverLink->readShort(), 0x0102);
}

TEST_F(LinkTest, CPUWriteAndReadWord) {
    cpuLink->writeWord(0x01020304);
    EXPECT_EQ(serverLink->readWord(), 0x01020304);
}

// Server named pipe on windows blocks on ConnectNamedPipe. Need better mechanism.
//TEST_F(LinkTest, ServerWriteAndReadWord) {
//    serverLink->writeWord(0x05060708);
//    EXPECT_EQ(cpuLink->readWord(), 0x05060708);
//}


using LinkPair = std::pair<Link *, Link *>;
typedef LinkPair* FactoryFunc();

class LinkPairTypedTest : public ::testing::TestWithParam<FactoryFunc*> {
    public:
    virtual ~LinkPairTypedTest() { delete pair;}

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
    std::atomic<bool> setupDone{false};
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

INSTANTIATE_TEST_CASE_P(
    ParameterisedLinkPairTest,
    LinkPairTypedTest,
    testing::Values(&FactoryFifo, &FactoryInMemory));

// FAILS on macos - permission denied - Could not open read FIFO /tmp/t800emul-read-0: Permission denied in Setup
TEST_P(LinkPairTypedTest, CPUWriteAndReadByte) {
    cpuLink->writeByte(16);
    EXPECT_EQ(serverLink->readByte(), 16);
}

TEST_P(LinkPairTypedTest, CPUWriteAndReadBytes) {
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

// FAILS ON WINDOWS 'Creating server named pipe` Could not create/open named pipe: Error 231. Throws in SetUp.
TEST_P(LinkPairTypedTest, CPUWriteAndReadShort) {
    m_thread = new std::thread([this] {
        cpuLink->writeShort(0x0102);
        finished();
    });

    EXPECT_EQ(serverLink->readShort(), 0x0102);
    waitForFinished();
}

// FAILS ON WINDOWS 'Creating server named pipe` Could not create/open named pipe: Error 231. Throws in SetUp.
TEST_P(LinkPairTypedTest, CPUWriteAndReadWord) {
    m_thread = new std::thread([this] {
        cpuLink->writeWord(0x01020304);
        finished();
    });

    EXPECT_EQ(serverLink->readWord(), 0x01020304);
    waitForFinished();
}

// Server named pipe on windows blocks on ConnectNamedPipe. Need better mechanism.
// FAILS ON WINDOWS 'Creating server named pipe` Could not create/open named pipe: Error 231. Throws in SetUp.
TEST_P(LinkPairTypedTest, ServerWriteAndReadByte) {
    m_thread = new std::thread([this] {
        serverLink->writeByte(32);
        finished();
    });

    EXPECT_EQ(cpuLink->readByte(), 32);
    waitForFinished();
}

// Server named pipe on windows blocks on ConnectNamedPipe. Need better mechanism.
// FAILS ON WINDOWS 'Creating server named pipe` Could not create/open named pipe: Error 231.  Throws in SetUp.
TEST_P(LinkPairTypedTest, ServerWriteAndReadWord) {
    m_thread = new std::thread([this] {
        serverLink->writeWord(0x05060708);
        finished();
    });

    EXPECT_EQ(cpuLink->readWord(), 0x05060708);
    waitForFinished();
}
