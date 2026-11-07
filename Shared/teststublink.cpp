//------------------------------------------------------------------------------
//
// File        : teststublink.cpp
// Description : Tests for the StubLink.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 07/11/2026
//
// (C) 2005-2026 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <vector>
#include <chrono>
#include <thread>

#include "gtest/gtest.h"
#include "link.h"
#include "stublink.h"
#include "constants.h"
#include "log.h"

class StubLinkTest : public ::testing::Test {
public:
    StubLinkTest() = default;
protected:

    void SetUp() override {
        setLogLevel(LOGLEVEL_DEBUG);
        logDebug("SetUp start");

        mStubLink = new StubLink(0, false);
        mStubLink->setDebug(true);
        logDebug("Setup complete");
        logFlush();
    }

    void TearDown() override {
        logDebug("TearDown start");
        delete mStubLink;
        logDebug("TearDown complete");
        logFlush();
    }

    StubLink *mStubLink = nullptr;
};

TEST_F(StubLinkTest, InitialConditions) {
    logDebug("InitialConditions start");
    EXPECT_EQ(mStubLink->getLinkNo(), 0);
    EXPECT_EQ(mStubLink->readAvailable(), false);
    EXPECT_EQ(mStubLink->writeAvailable(), true);
    logDebug("InitialConditions end");
}

TEST_F(StubLinkTest, ReadAvailableTransitions) {
    logDebug("ReadAvailableTransitions start");
    std::vector<BYTE8> frameVector;
    frameVector.push_back(0x01);
    frameVector.push_back(0x02);
    mStubLink->setReadableBytes(frameVector);
    EXPECT_EQ(mStubLink->readAvailable(), true);
    EXPECT_EQ(mStubLink->readByte(), 0x01);
    EXPECT_EQ(mStubLink->readAvailable(), true);
    EXPECT_EQ(mStubLink->readByte(), 0x02);
    EXPECT_EQ(mStubLink->readAvailable(), false);
    logDebug("ReadAvailableTransitions end");
}

TEST_F(StubLinkTest, GetFirstWrittenByte) {
    logDebug("GetFirstWrittenByte start");
    EXPECT_EQ(mStubLink->getFirstWrittenByte(), 0x00); // with a warning - this violates the precondition
    mStubLink->writeByte(0x01);
    mStubLink->writeByte(0x02);
    EXPECT_EQ(mStubLink->getFirstWrittenByte(), 0x01);
    EXPECT_EQ(mStubLink->getFirstWrittenByte(), 0x02);
    EXPECT_EQ(mStubLink->getFirstWrittenByte(), 0x00); // with a warning - this violates the precondition
    logDebug("GetFirstWrittenByte end");
}

TEST_F(StubLinkTest, WriteAvailableTransitions) {
    logDebug("WriteAvailableTransitions start");
    mStubLink->setWriteQueueSize(2);
    EXPECT_EQ(mStubLink->writeAvailable(), true); // initial conditions should still hold
    mStubLink->writeByte(0x01);
    EXPECT_EQ(mStubLink->writeAvailable(), true);
    mStubLink->writeByte(0x02);
    EXPECT_EQ(mStubLink->writeAvailable(), false); // queue now juste full
    mStubLink->writeByte(0x03); // more in queue than the queue size
    EXPECT_EQ(mStubLink->writeAvailable(), false); // over full
    EXPECT_EQ(mStubLink->getFirstWrittenByte(), 0x01); // now just full
    EXPECT_EQ(mStubLink->writeAvailable(), false);
    EXPECT_EQ(mStubLink->getFirstWrittenByte(), 0x02); // now got space
    EXPECT_EQ(mStubLink->writeAvailable(), true);
    logDebug("WriteAvailableTransitions end");
}
