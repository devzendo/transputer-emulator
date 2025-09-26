//------------------------------------------------------------------------------
//
// File        : testasynclink.cpp
// Description : Tests for asynchronous link abstraction
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 26/09/2025
//
// (C) 2005-2025 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "link.h"
#include "asynclink.h"
#include "linkfactory.h"
#include "log.h"

class AsyncLinkTest : public ::testing::Test {
protected:

    void SetUp() override {
        setLogLevel(LOGLEVEL_DEBUG);
        logDebug("SetUp start");

        logDebug("Creating Link A");
        linkA = new AsyncLink(0, false);
        linkA->setDebug(true);
        logDebug("Initialising Link A");
        linkA->initialise();

        logDebug("Creating Link B");
        linkB = new AsyncLink(0, false);
        linkB->setDebug(true);
        logDebug("Initialising Link B");
        linkB->initialise();

        logDebug("Setup complete");
        logFlush();
    }

    void TearDown() override {
        logDebug("TearDown start");
        if (linkA != nullptr) {
            logDebug("Resetting Link A");
            linkA->resetLink();
	        delete linkA;
        }
        if (linkB != nullptr) {
            logDebug("Resetting Link B");
            linkB->resetLink();
	        delete linkB;
        }
        logDebug("TearDown complete");
        logFlush();
    }

    AsyncLink *linkA = nullptr;
    AsyncLink *linkB = nullptr;
};

TEST_F(AsyncLinkTest, WriteAndReadByte) {
    linkA->writeByte(16);
    EXPECT_EQ(linkB->readByte(), 16);
}

TEST_F(AsyncLinkTest, WriteAndReadBytes) {
    BYTE8 writeBuf[4] = { 0xff, 0x7f, 0x60, 0x21 };
    int bytesWritten = linkA->writeBytes(writeBuf, 4);
    EXPECT_EQ(bytesWritten, 4);

    BYTE8 readBuf[4];
    int bytesRead = linkB->readBytes(readBuf, 4);
    EXPECT_EQ(bytesRead, 4);

    EXPECT_EQ(readBuf[0], 0xff);
    EXPECT_EQ(readBuf[1], 0x7f);
    EXPECT_EQ(readBuf[2], 0x60);
    EXPECT_EQ(readBuf[3], 0x21);
}
