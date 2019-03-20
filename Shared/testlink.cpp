//------------------------------------------------------------------------------
//
// File        : testlink.cpp
// Description : Tests for link abstraction
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 19/03/2019
//
// (C) 2005-2019 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "link.h"
#include "linkfactory.h"

class LinkTest : public ::testing::Test {
protected:
    void SetUp() override {
        cpuLinkFactory = new LinkFactory(false, true);
        serverLinkFactory = new LinkFactory(true, true);
        cpuLink = cpuLinkFactory->createLink(0);
        serverLink = serverLinkFactory->createLink(0);
        cpuLink->setDebug(true);
        cpuLink->initialise();
        serverLink->setDebug(true);
        serverLink->initialise();
    }

    void TearDown() override {
        cpuLink->resetLink();
        serverLink->resetLink();
    }

    LinkFactory *cpuLinkFactory;
    LinkFactory *serverLinkFactory;
    Link *cpuLink;
    Link *serverLink;
};

// Note that links are currently blocking, and synchronous. If more than PIPE_BUF bytes are written, write() will block
// - see man 7 pipe.

TEST_F(LinkTest, CPUWriteAndReadByte) {
    cpuLink->writeByte(16);
    EXPECT_EQ(serverLink->readByte(), 16);
}

TEST_F(LinkTest, ServerWriteAndReadByte) {
    serverLink->writeByte(32);
    EXPECT_EQ(cpuLink->readByte(), 32);
}

TEST_F(LinkTest, CPUWriteAndReadWord) {
    cpuLink->writeWord(0x01020304);
    EXPECT_EQ(serverLink->readWord(), 0x01020304);
}

TEST_F(LinkTest, ServerWriteAndReadWord) {
    serverLink->writeWord(0x05060708);
    EXPECT_EQ(cpuLink->readWord(), 0x05060708);
}
