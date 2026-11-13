//------------------------------------------------------------------------------
//
// File        : testcpu.cpp
// Description : Tests certain aspects of the CPU, with stub links and memory.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 13/11/2026
//
// (C) 2005-2026 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <thread>
#include <atomic>

#include "gtest/gtest.h"
using namespace std;
#include "cpu.h"
#include "boot.h"
#include "stublink.h"
#include "log.h"
#include "memory.h"
#include "types.h"
#include "memloc.h"

WORD32 flags;

#include "flags.h"

class CPUTest : public ::testing::Test {
public:
    CPUTest()  {
    }
protected:
    Link *myLinks[4];
    Memory *myMemory = nullptr;
    SymbolTable *mySymbolTable = nullptr;
    CPU *myCPU = nullptr;

    void SetUp() override {
        setLogLevel(LOGLEVEL_DEBUG);
        SET_FLAGS(DebugFlags_LinkComms | MemAccessDebug_Full | DebugFlags_TerminateOnMemViol | Debug_Disasm);
        logDebug("SetUp start");
        myMemory = new Memory();
        if (!myMemory->initialise(1024)) {
            logError("Memory initialisation failed");
            FAIL();
        }
        mySymbolTable = new SymbolTable();
        myMemory->initialiseROMFileAndSymbolTable(nullptr, mySymbolTable);

        logDebug("Link setup");
        myLinks[0] = new StubLink(0, false);
        myLinks[0]->setDebug(true);
        myLinks[0]->initialise();
        myLinks[1] = new StubLink(0, false);
        myLinks[1]->setDebug(true);
        myLinks[1]->initialise();
        myLinks[2] = nullptr;
        myLinks[3] = nullptr;
        myCPU = new CPU();
        myCPU->initialiseSymbolTable(mySymbolTable);
        myCPU->initialise(myMemory, myLinks);

        logDebug("Setup complete");
        logFlush();
        // Each test needs to initialise the program to run, by setting the boot data in myLinks[0].
        // Then calling myCPU->_emulate_prologue(false);
    }

    void TearDown() override {
        logDebug("^^^^^ TEST ^^^^^");
        myCPU->_emulate_epilogue();
        logDebug("TearDown start");
        // notify?
        for (int i = 0; i < 2; i++) {
            myLinks[i]->resetLink();
            // CPU dtor deletes links
        }
        delete myMemory;
        delete myCPU;
        delete mySymbolTable;
        logDebug("TearDown complete");
        logFlush();
    }

    void boot_with(const std::initializer_list<BYTE8> bytes) const {
        logDebug("vvvvv BOOT vvvvv");
        std::vector<BYTE8> byte8s = bytes;
        // How many bytes to boot with? Must be >= 2.
        byte8s.insert(byte8s.begin(), byte8s.size());
        reinterpret_cast<StubLink *>(myLinks[0])->setReadableBytes(byte8s);
        myCPU->_emulate_prologue(false);
        logDebug("vvvvv TEST vvvvv");
    }

    void boot_with_terminate() const {
        boot_with({
            0x2C, 0xF2, // terminate
        });
    }
};

TEST_F(CPUTest, InitialConditions) {
    boot_with_terminate();
    // Don't start interpreting, just see what's set up after boot from link with a 2 byte program.
    // TODO change CPUTest to be a FRIEND_TEST
}

TEST_F(CPUTest, Terminates) {
    boot_with_terminate();
    myCPU->_emulate_loop(); // this runs until it terminates
    // Got here? Good! It interpreted the terminate instruction and set the terminate flag.
}
