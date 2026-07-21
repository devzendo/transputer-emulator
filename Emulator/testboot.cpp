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
#if defined(PLATFORM_WINDOWS)
#include "namedpipelink.h"
#elif defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
#include "fifolink.h"
#endif
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
    Boot *myBoot = nullptr;
    std::thread *m_thread = nullptr;
    std::atomic<bool> setupDone{false};
    std::atomic<bool> done{false};
    std::atomic<int> bootLen{0};


    void SetUp() override {
        setLogLevel(LOGLEVEL_DEBUG);
        SET_FLAGS(DebugFlags_LinkComms | MemAccessDebug_Full | DebugFlags_TerminateOnMemViol);
        logDebug("SetUp start");
	    myMemory = new Memory();
        if (!myMemory->initialise(1024)) {
            logError("Memory initialisation failed");
            FAIL();
        }
	    SymbolTable * symbolTable = new SymbolTable();
        myMemory->initialiseROMFileAndSymbolTable(nullptr, symbolTable);

        logDebug("Link setup");
        for (int i = 0; i < 4; i++) {
#if defined(PLATFORM_WINDOWS)
            myControlLinks[i] = new NamedPipeLink(i, true);
#elif defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
            myControlLinks[i] = new FIFOLink(i, true);
#endif
            myControlLinks[i]->initialise();
#if defined(PLATFORM_WINDOWS)
            myBootLinks[i] = new NamedPipeLink(i, false);
#elif defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
            myBootLinks[i] = new FIFOLink(i, false);
#endif
            myBootLinks[i]->setDebug(true);
            myBootLinks[i]->initialise();
        }

        logDebug("Setup complete");
        logFlush();
        logDebug("vvvvv TEST vvvvv");
    }

    void startBoot() {
        logDebug("Boot setup waiting for initial setup");
        while (!setupDone.load(std::memory_order_acquire)) {
            std::this_thread::yield();
        }
        logDebug("Initial setup done; booting...");
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
            logDebug("Boot finished");
            bootLen.store(myBoot->bootLen(), std::memory_order_release);
            logDebug("Boot thread ending");
            done.store(true, std::memory_order_release);
        });
    }

    void waitUntilEndOfBoot() {
        while (!done.load(std::memory_order_acquire)) {
            std::this_thread::yield();
        }
        if (m_thread != nullptr) {
            logDebug("Joining thread");
            m_thread->join();
            logDebug("Thread joined");
        }
    }

    void TearDown() override {
        logDebug("^^^^^ TEST ^^^^^");
        logDebug("TearDown start");
        // notify?
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

    void littleSleep() {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
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

TEST_F(PeekPokeBootTest, PeekAWordFromLegalMemory) {
    myMemory->setWord(MemStart + 20, 0xCAFEF00D);

    setupDone.store(true, std::memory_order_release);
    startBoot();

    myControlLinks[0]->writeByte(BOOT_PEEK);
    myControlLinks[0]->writeWord(MemStart + 20);

    terminateBootLoop();
    waitUntilEndOfBoot();
    WORD32 word = myControlLinks[0]->readWord();
    EXPECT_EQ(word, 0xCAFEF00D);
}

TEST_F(PeekPokeBootTest, PeekAWordFromOutsideLegalMemory) {
    setupDone.store(true, std::memory_order_release);
    startBoot();

    myControlLinks[0]->writeByte(BOOT_PEEK);
    myControlLinks[0]->writeWord(MemStart + 1025);

    littleSleep();

    terminateBootLoop();
    waitUntilEndOfBoot();
    WORD32 word = myControlLinks[0]->readWord();
    EXPECT_EQ(word, 0xDEADF00D);
}

TEST_F(PeekPokeBootTest, PokeAWordToLegalMemory) {
    setupDone.store(true, std::memory_order_release);
    startBoot();

    myControlLinks[0]->writeByte(BOOT_POKE);
    myControlLinks[0]->writeWord(MemStart + 20);
    myControlLinks[0]->writeWord(0x12345678);

    littleSleep();

    terminateBootLoop();
    waitUntilEndOfBoot();
    WORD32 word = myMemory->getWord(MemStart + 20);
    EXPECT_EQ(word, 0x12345678);
}

// We are setting the flags so that memory violations cause the terminate
// flag to be set. The boot code will not allow writes outside memory, so
// this attempt will NOT cause the terminate flag to be set, but cannot
// otherwise be detected- a warning diag is reported. Manually observe this:
TEST_F(PeekPokeBootTest, PokeAWordOutsideLegalMemory) {
    setupDone.store(true, std::memory_order_release);
    startBoot();

    myControlLinks[0]->writeByte(BOOT_POKE);
    myControlLinks[0]->writeWord(MemStart + 1025);
    myControlLinks[0]->writeWord(0x12345678);

    littleSleep();

    terminateBootLoop();
    waitUntilEndOfBoot();
    EXPECT_EQ(IS_FLAG_SET(EmulatorState_Terminate), false);
}

TEST_F(PeekPokeBootTest, InitialConditions) {
    logDebug("Start of InitialConditions");

    logDebug("Starting booting InitialConditions");

    setupDone.store(true, std::memory_order_release);
    startBoot();
    logDebug("Terminating boot loop");

    terminateBootLoop(); // sends a minimum 2-byte boot file
    logDebug("Waiting a bit");

    littleSleep();
    logDebug("Waiting for end of boot");
    waitUntilEndOfBoot();
    EXPECT_EQ(bootLen.load(), 2);
}

TEST_F(PeekPokeBootTest, BootIt) {
    setupDone.store(true, std::memory_order_release);
    startBoot();

    int boot_length = 7;
    myControlLinks[0]->writeByte(boot_length);
    myControlLinks[0]->writeByte(0x00);
    myControlLinks[0]->writeByte(0x01);
    myControlLinks[0]->writeByte(0x02);
    myControlLinks[0]->writeByte(0x03);
    myControlLinks[0]->writeByte(0x04);
    myControlLinks[0]->writeByte(0x05);
    myControlLinks[0]->writeByte(0x06);

    littleSleep();
    waitUntilEndOfBoot();

    // The boot loop will have terminated.
    EXPECT_EQ(myMemory->getByte(MemStart + 0), 0x00);
    EXPECT_EQ(myMemory->getByte(MemStart + 1), 0x01);
    EXPECT_EQ(myMemory->getByte(MemStart + 2), 0x02);
    EXPECT_EQ(myMemory->getByte(MemStart + 3), 0x03);
    EXPECT_EQ(myMemory->getByte(MemStart + 4), 0x04);
    EXPECT_EQ(myMemory->getByte(MemStart + 5), 0x05);
    EXPECT_EQ(myMemory->getByte(MemStart + 6), 0x06);

    EXPECT_EQ(myMemory->getByte(MemStart + 7), 0x00);

    EXPECT_EQ(myBoot->bootLen(), 7);
}
