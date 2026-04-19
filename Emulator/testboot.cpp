//------------------------------------------------------------------------------
//
// File        : testboot.cpp
// Description : Tests the peek/poke boot protocol over links to memory.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 19/04/2026
//
// (C) 2005-2026 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------


#include "gtest/gtest.h"
using namespace std;
#include "boot.h"
#include "fifolink.h"
#include "log.h"
#include "memory.h"
#include "types.h"

WORD32 flags;

class PeekPokeBootTest : public ::testing::Test {
public:
    PeekPokeBootTest()  {
    }
protected:

    Memory *myMemory;
    Link *myBootLinks[4];
    Link *myControlLinks[4];
    Boot *myBoot;

    void SetUp() override {
        setLogLevel(LOGLEVEL_DEBUG);
        logDebug("SetUp start");
	    myMemory = new Memory();
        if (!myMemory->initialise(1024)) {
            logError("Memory initialisation failed");
            FAIL();
        }
        for (int i = 0; i < 4; i++) {
            myControlLinks[i] = new FIFOLink(i, true);
            myBootLinks[i] = new FIFOLink(i, false);
        }
        myBoot = new Boot();
        myBoot->initialise(myMemory, myBootLinks);
        // It reads from the 'client' links, this test writes to the 'control' (server) links.

        logDebug("Setup complete");
        logFlush();
    }

    void TearDown() override {
        logDebug("TearDown start");
        for (int i = 0; i < 4; i++) {
            myBootLinks[i]->resetLink();
            delete myBootLinks[i];
            myControlLinks[i]->resetLink();
            delete myControlLinks[i];
        }
        delete myMemory;
        delete myBoot;
        logDebug("TearDown complete");
        logFlush();
    }
};

TEST_F(PeekPokeBootTest, InitialConditions) {
    // EXPECT_EQ(handler.counter(), 0);
    // EXPECT_EQ(handler.is_running(), false);
}
