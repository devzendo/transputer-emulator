//------------------------------------------------------------------------------
//
// File        : testsystemlink.cpp
// Description : Tests for the SystemLink (using StubLink to drive it) - the
//               functionality that's shared by all subclasses of SystemLink.
//               Also see teststublink for tests checking the stubbing behaviour.
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

class SystemLinkTest : public ::testing::Test {
public:
    SystemLinkTest() = default;
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

// Other initial conditions are tested in teststublink.
TEST_F(SystemLinkTest, InitialRegisterConditions) {
    logDebug("InitialRegisterConditions start");
    EXPECT_EQ(mStubLink->m_receive_registers.m_data_pointer, nullptr);
    EXPECT_EQ(mStubLink->m_receive_registers.m_length, 0);
    EXPECT_EQ(mStubLink->m_receive_registers.m_workspace_pointer, 0);
    EXPECT_EQ(mStubLink->readComplete(), NotProcess_p);

    EXPECT_EQ(mStubLink->m_send_registers.m_data_pointer, nullptr);
    EXPECT_EQ(mStubLink->m_send_registers.m_length, 0);
    EXPECT_EQ(mStubLink->m_send_registers.m_workspace_pointer, 0);
    EXPECT_EQ(mStubLink->writeComplete(), NotProcess_p);
    logDebug("InitialRegisterConditions end");
}

TEST_F(SystemLinkTest, AsyncReadRegisterConditions) {
    logDebug("AsyncReadRegisterConditions start");
    char readBuffer[] = "ABCDEF";
    mStubLink->readDataAsync(0xDEADBEEF, reinterpret_cast<BYTE8 *>(&readBuffer[0]), 4);
    EXPECT_EQ(mStubLink->m_receive_registers.m_data_pointer, reinterpret_cast<BYTE8 *>(&readBuffer[0]));
    EXPECT_EQ(mStubLink->m_receive_registers.m_length, 4);
    EXPECT_EQ(mStubLink->m_receive_registers.m_workspace_pointer, 0xDEADBEEF);
    // Also doesn't affect send registers...
    EXPECT_EQ(mStubLink->m_send_registers.m_data_pointer, nullptr);
    EXPECT_EQ(mStubLink->m_send_registers.m_length, 0);
    EXPECT_EQ(mStubLink->m_send_registers.m_workspace_pointer, 0);
    logDebug("AsyncReadRegisterConditions end");
}

TEST_F(SystemLinkTest, AsyncWriteWhenFull) {
    logDebug("AsyncWriteWhenFull start");
    char writeBuffer[] = "ABCDEF";
    mStubLink->setWriteQueueSize(0);
    EXPECT_EQ(mStubLink->writeDataAsync(0xDEADBEEF, reinterpret_cast<BYTE8 *>(&writeBuffer[0]), 4), false);
    // And the registers aren't set up
    EXPECT_EQ(mStubLink->m_send_registers.m_data_pointer, nullptr);
    EXPECT_EQ(mStubLink->m_send_registers.m_length, 0);
    EXPECT_EQ(mStubLink->m_send_registers.m_workspace_pointer, 0);
}

TEST_F(SystemLinkTest, AsyncWriteWhenSpaceRemains) {
    logDebug("AsyncWriteWhenSpaceRemains start");
    char writeBuffer[] = "ABCDEF";
    mStubLink->setWriteQueueSize(1);
    EXPECT_EQ(mStubLink->writeDataAsync(0xDEADBEEF, reinterpret_cast<BYTE8 *>(&writeBuffer[0]), 1), true);
    logDebug("AsyncWriteWhenSpaceRemains end");
}

TEST_F(SystemLinkTest, AsyncWriteRegisterConditions) {
    logDebug("AsyncWriteRegisterConditions start");
    char writeBuffer[] = "ABCDEF";
    EXPECT_EQ(mStubLink->writeDataAsync(0xDEADBEEF, reinterpret_cast<BYTE8 *>(&writeBuffer[0]), 4), true);
    EXPECT_EQ(mStubLink->m_send_registers.m_data_pointer, reinterpret_cast<BYTE8 *>(&writeBuffer[0]));
    EXPECT_EQ(mStubLink->m_send_registers.m_length, 4);
    EXPECT_EQ(mStubLink->m_send_registers.m_workspace_pointer, 0xDEADBEEF);
    // Also doesn't affect receive registers
    EXPECT_EQ(mStubLink->m_receive_registers.m_data_pointer, nullptr);
    EXPECT_EQ(mStubLink->m_receive_registers.m_length, 0);
    EXPECT_EQ(mStubLink->m_receive_registers.m_workspace_pointer, 0);
    logDebug("AsyncWriteRegisterConditions end");
}

