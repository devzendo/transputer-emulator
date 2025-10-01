//------------------------------------------------------------------------------
//
// File        : testlink.cpp
// Description : Tests for link abstraction
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 19/03/2019
//
// (C) 2005-2025 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

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
