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

#include <thread>

#include "gtest/gtest.h"
using namespace std;
#include "boot.h"
#include "fifolink.h"
#include "log.h"
#include "memory.h"
#include "types.h"
#include "memloc.h"

WORD32 flags;
#include "flags.h"

class PeekPokeBootTest : public ::testing::Test {
public:
    PeekPokeBootTest()  {
    }
protected:

    Memory *myMemory;
    Link *myBootLinks[4];
    Link *myControlLinks[4];
    Boot *myBoot;
    std::thread *m_thread = nullptr;

    void SetUp() override {
        setLogLevel(LOGLEVEL_DEBUG);
        SET_FLAGS(DebugFlags_LinkComms);
        logDebug("SetUp start");
	    myMemory = new Memory();
        if (!myMemory->initialise(1024)) {
            logError("Memory initialisation failed");
            FAIL();
        }
        logDebug("Link setup");
        for (int i = 0; i < 4; i++) {
            // This'll have to change to NamedPipeLink for Windows.
            myControlLinks[i] = new FIFOLink(i, true);
            myControlLinks[i]->initialise();
            myBootLinks[i] = new FIFOLink(i, false);
            myBootLinks[i]->initialise();
        }
        logDebug("Boot setup");
        myBoot = new Boot();
        myBoot->initialise(myMemory, myBootLinks);
        // It reads from the 'client' links, this test writes to the 'control' (server) links.

        // Boot has to run in a separate thread as these are synchronous links that boot will read without sensing
        // that they have any data (current behaviour) and this will block. So if we call start, the first byte read
        // will block... and the test setup will never be able to send the peek/poke/boot.
        logDebug("Starting thread");
        m_thread = new std::thread([this] {
            logDebug("Thread started, booting...");
            myBoot->start();
            logDebug("Boot finished, thread ending");
        });

        logDebug("Setup complete");
        logFlush();
    }

    void TearDown() override {
        logDebug("TearDown start");
        // notify?
        if (m_thread != nullptr) {
            logDebug("Joining thread");
            m_thread->join();
            logDebug("Thread joined");
        }
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

    void terminateBootLoop() {
        logDebug("Terminating boot loop");
        // Terminate boot loop by booting:
        myControlLinks[0]->writeByte(2); // Shortest boot code
        myControlLinks[0]->writeByte(0xDE);
        myControlLinks[0]->writeByte(0xAD);
        logDebug("Boot loop should be ended now");
    }
};

// For characterising the initial code lifted out of main, we only use Link0.
// With this initial code, synchronous links don't currently provide a mechanism for
// sensing whether any data is available. Allowing any / all links is an enhancement.

TEST_F(PeekPokeBootTest, PeekAByte) {
    myMemory->setByte(MemStart, 0x69);

    myControlLinks[0]->writeByte(BOOT_PEEK);
    myControlLinks[0]->writeWord(MemStart);

    BYTE8 byte = myControlLinks[0]->readByte();

    terminateBootLoop();
    EXPECT_EQ(byte, 0x69);
}
