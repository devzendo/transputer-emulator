//------------------------------------------------------------------------------
//
// File        : testinmemorylink.cpp
// Description : Tests for the InMemoryLink.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 08/07/2026
//
// (C) 2005-2026 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <vector>
#include <atomic>
#include <chrono>
#include <thread>

#include "gtest/gtest.h"
#include "link.h"
#include "inmemorylink.h"
#include "constants.h"
#include "log.h"
#include "misc.h"

class InMemoryLinkTest : public ::testing::Test {
public:
    InMemoryLinkTest() {
    }
protected:

    void SetUp() override {
        setLogLevel(LOGLEVEL_DEBUG);
        logDebug("SetUp start");

        logDebug("Setup complete");
        logFlush();
    }

    void TearDown() override {
        logDebug("TearDown start");

        logDebug("TearDown complete");
        logFlush();
    }
};

TEST_F(InMemoryLinkTest, InitialConditions) {
    EXPECT_EQ(0, 0);
}

