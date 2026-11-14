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

#include "constants.h"
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
        SET_FLAGS(DebugFlags_LinkComms | MemAccessDebug_Full | DebugFlags_TerminateOnMemViol | Debug_Disasm
            | DebugFlags_TerminateOnMemViol);
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

    void interpret_until(WORD32 iPtrReaches) const {
        logDebugF("Interpreting until 0x%08X reached...", iPtrReaches);
        while (myCPU->IPtr != iPtrReaches && IS_FLAG_CLEAR(EmulatorState_Terminate)) {
            myCPU->interpret();
        }
        logDebugF("IPtr has reached 0x%08X, end of interpretation", iPtrReaches);
    }

    void boot_with_typical_setup() const {
        // Standard Transputer startup code, taken from the macro assembler bootloader.
        // This is 0x8A bytes long.
        boot_with({
            // Initialisation
            0x24, 0xF2,
            0x21, 0xF8,
            0x24, 0xF2,
            0x21, 0xFC,
            0x24, 0xF2,
            0x67, 0x20, 0x20, 0x20,
            0x20, 0x20, 0x22, 0x44,
            0xE0,
            0x24, 0xF2,
            0x67, 0x20, 0x20, 0x20,
            0x20, 0x20, 0x22, 0x48,
            0xE0,
            0x40,
            0x25, 0xF4,
            0x22, 0xF9,
            0x25, 0xF7,
            0x29, 0xFC,
            0x24, 0xF2,
            0x67, 0x20, 0x20, 0x20,
            0x20, 0x20, 0x22, 0x40,
            0xE0,
            0x24, 0xF2,
            0x67, 0x20, 0x20, 0x20,
            0x20, 0x20, 0x21, 0x4C,
            0xE0,
            0x24, 0xF2,
            0x67, 0x20, 0x20, 0x20,
            0x20, 0x20, 0x21, 0x48,
            0xE0,
            0x24, 0xF2,
            0x67, 0x20, 0x20, 0x20,
            0x20, 0x20, 0x21, 0x44,
            0xE0,
            0x24, 0xF2,
            0x67, 0x20, 0x20, 0x20,
            0x20, 0x20, 0x21, 0x40,
            0xE0,
            0x24, 0xF2,
            0x67, 0x20, 0x20, 0x20,
            0x20, 0x20, 0x20, 0x4C,
            0xE0,
            0x24, 0xF2,
            0x67, 0x20, 0x20, 0x20,
            0x20, 0x20, 0x20, 0x48,
            0xE0,
            0x24, 0xF2,
            0x67, 0x20, 0x20, 0x20,
            0x20, 0x20, 0x20, 0x44,
            0xE0,
            0x24, 0xF2,
            0x67, 0x20, 0x20, 0x20,
            0x20, 0x20, 0x20, 0x40,
            0xE0, // at 0x800000F9
        });
    }
};

TEST_F(CPUTest, InitialConditions) {
    boot_with_terminate();
    // Don't start interpreting, just see what's set up after boot from link with a 2 byte terminate program.
    myCPU->DumpRegs(LOGLEVEL_DEBUG);
    myCPU->DumpQueueRegs(LOGLEVEL_DEBUG);
    EXPECT_EQ(myCPU->IPtr, MemStart);
    EXPECT_EQ(myCPU->Wdesc, (MemStart + 4) | 0x00000001); // low priority
    EXPECT_EQ(myCPU->HiHead, 0xDEADF00D);
    EXPECT_EQ(myCPU->HiTail, 0xDEADF00D);
    EXPECT_EQ(myCPU->LoHead, 0xDEADF00D);
    EXPECT_EQ(myCPU->LoTail, 0xDEADF00D);

    EXPECT_EQ(myMemory->getByte(MemStart + 0), 0x2C); // Terminate
    EXPECT_EQ(myMemory->getByte(MemStart + 1), 0xF2);
}

TEST_F(CPUTest, Terminates) {
    boot_with_terminate();
    myCPU->_emulate_loop(); // this runs until it terminates
    // Got here? Good! It interpreted the terminate instruction and set the terminate flag.
}

TEST_F(CPUTest, InitialConditionsAfterTypicalSetup) {
    boot_with_typical_setup();
    interpret_until(0x800000FA);
    myCPU->DumpRegs(LOGLEVEL_DEBUG);
    myCPU->DumpQueueRegs(LOGLEVEL_DEBUG);
	myCPU->DumpClockRegs(LOGLEVEL_DEBUG, myCPU->InstCycles + myCPU->MemCycles);

    EXPECT_EQ(myCPU->IPtr, 0x800000FA);
    EXPECT_EQ(myCPU->Wdesc, (WordAlign(MemStart + 0x8A)) | 0x00000001); // low priority => 0x800000FD
    EXPECT_EQ(myCPU->HiHead, NotProcess_p);
    EXPECT_EQ(myCPU->HiTail, 0xDEADF00D);
    EXPECT_EQ(myCPU->LoHead, NotProcess_p);
    EXPECT_EQ(myCPU->LoTail, 0xDEADF00D);
}
