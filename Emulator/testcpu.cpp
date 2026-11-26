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
    CPUTest()  = default;
protected:
    Link *myLinks[4]{};
    Memory *myMemory = nullptr;
    SymbolTable *mySymbolTable = nullptr;
    CPU *myCPU = nullptr;

    void SetUp() override {
        setLogLevel(LOGLEVEL_DEBUG);
        flags = 0;
        logDebug("SetUp start");
        myMemory = new Memory();
        if (!myMemory->initialise(2048)) {
            logError("Memory initialisation failed");
            FAIL();
        }
        mySymbolTable = new SymbolTable();
        myMemory->initialiseROMFileAndSymbolTable(nullptr, mySymbolTable);

        logDebug("Link setup");
        myLinks[0] = new StubLink(0, false);
        myLinks[0]->initialise();
        myLinks[1] = new StubLink(0, false);
        myLinks[1]->initialise();
        // The links will have their debug flag set near when we need to investigate.
        myLinks[2] = nullptr;
        myLinks[3] = nullptr;
        myCPU = new CPU();
        myCPU->initialiseSymbolTable(mySymbolTable);
        myCPU->initialise(myMemory, myLinks);
        logDebug("CPU setup complete - initial registers:");
        myCPU->DumpRegs(LOGLEVEL_DEBUG);
        myCPU->DumpQueueRegs(LOGLEVEL_DEBUG);

        logDebug("Setup complete");
        logFlush();
        // Each test needs to initialise the program to run, by setting the boot data in myLinks[0].
        // Then calling myCPU->_emulate_prologue(false);
    }

    void TearDown() override {
        logDebug("^^^^^ TEST ^^^^^");
        logDebug("Registers at end of test:");
        myCPU->DumpRegs(LOGLEVEL_DEBUG);
        myCPU->DumpQueueRegs(LOGLEVEL_DEBUG);
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
        std::vector<BYTE8> byte8s = bytes;
        boot_with_vector(byte8s);
    }

    void boot_with_vector(std::vector<BYTE8> bytes) const {
        logDebug("vvvvv BOOT vvvvv");
        // How many bytes to boot with? Must be >= 2.
        bytes.insert(bytes.begin(), bytes.size());
        reinterpret_cast<StubLink *>(myLinks[0])->setReadableBytes(bytes);
        myCPU->_emulate_prologue(false);
        logDebug("vvvvv TEST vvvvv");
        SET_FLAGS(DebugFlags_LinkComms | MemAccessDebug_Full | DebugFlags_TerminateOnMemViol | Debug_DisRegs
            | DebugFlags_TerminateOnMemViol);
        myLinks[0]->setDebug(true);
        myLinks[1]->setDebug(true);
    }

    void boot_with_terminate() const {
        boot_with({
            0x2C, 0xF2, // terminate
        });
    }

    void interpret_until(const WORD32 iPtrReaches) const {
        logDebugF("Interpreting until 0x%08X reached...", iPtrReaches);
        while (myCPU->IPtr != iPtrReaches && IS_FLAG_CLEAR(EmulatorState_Terminate)) {
            myCPU->interpret();
        }
        logDebugF("IPtr has reached 0x%08X, end of interpretation", iPtrReaches);
    }

    void boot_with_typical_setup(const std::initializer_list<BYTE8> rest_of_setup) const {
        // Standard Transputer startup code, taken from the macro assembler bootloader.
        // This is 0x8A bytes long, before you add 'rest_of_setup'.
        const std::vector<BYTE8> typical_setup = {
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
        };
        std::vector<BYTE8> byte8s = typical_setup;
        byte8s.insert( byte8s.end(), rest_of_setup.begin(), rest_of_setup.end() );
        boot_with_vector(byte8s);
    }

    void boot_with_string_out_to_link() const {
        boot_with_typical_setup({
            // 800000FA MAIN:
            0x21, 0x20, 0xB0,        // 800000FA ajw     0x100    ; allow for 64 stack frames
            0x4C,                    // 800000FD ldc     HWSTR - _M1
            0x21, 0xFB,              // 800000FE ldpi
            // 80000100 _M1:
            0x67, 0x20, 0x20, 0x20,  // 80000100 ldc     LINK0_OUTPUT
            0x20, 0x20, 0x20, 0x40,  // 80000104
            0x4B,                    // 80000108 ldc     11
            0xFB,                    // 80000109 out
            0x22, 0x4A,              // 8000010A ldc 42
            // 8000010C HWSTR:
            0x68, 0x65, 0x6C, 0x6C,  // 8000010C DB      "hello world", 0x00 ; length 11
            0x6F, 0x20, 0x77, 0x6F,  // 80000110
            0x72, 0x6C, 0x64, 0x00,  // 80000114
        });
    }

    void boot_with_string_out_to_memory() const {
        boot_with_typical_setup({
            // 800000FA MAIN:
            0x21, 0x20, 0xB0,        // 800000FA ajw     0x100    ; allow for 64 stack frames
            0x24, 0xF2,              // 800000FD mint             ; initialise channel
            0x21, 0x48,              // 800000FF ldc     CHAN - _M1
            0x21, 0xFB,              // 80000101 ldpi
            // 80000103 _M1:
            0xE0,                    // 80000103 stnl    0
            0x48,                    // 80000104 ldc     HWSTR - _M2 ; Output string data to channel
            0x21, 0xFB,              // 80000105 ldpi
            // 80000107 _M2:
            0x21, 0x40,              // 80000107 ldc     CHAN - _M3
            0x21, 0xFB,              // 80000109 ldpi
            // 8000010B _M3:
            0x4B,                    // 8000010B ldc     11
            0xFB,                    // 8000010C out
            0x22, 0x4A,              // 8000010D ldc 42  ; nop-ish
            // 8000010F HWSTR:
            0x68, 0x65, 0x6C, 0x6C,  // 8000010F DB      "hello world", 0x00 ; length 11
            0x6F, 0x20, 0x77, 0x6F,  // 80000113
            0x72, 0x6C, 0x64, 0x00,  // 80000117
            // 8000011B CHAN:
            0x0000,                  // 8000011B DW      0
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
    boot_with_typical_setup({});
    interpret_until(0x800000FA);
    myCPU->DumpRegs(LOGLEVEL_DEBUG);
    myCPU->DumpQueueRegs(LOGLEVEL_DEBUG);
	myCPU->DumpClockRegs(LOGLEVEL_DEBUG, myCPU->InstCycles + myCPU->MemCycles);

    EXPECT_EQ(myCPU->IPtr, 0x800000FA);        // vvvv typical setup is 0x8A bytes
    EXPECT_EQ(myCPU->Wdesc, (WordAlign(MemStart + 0x8A)) | 0x00000001); // low priority => 0x800000FD
    EXPECT_EQ(myCPU->HiHead, NotProcess_p);
    EXPECT_EQ(myCPU->HiTail, 0xDEADF00D);
    EXPECT_EQ(myCPU->LoHead, NotProcess_p);
    EXPECT_EQ(myCPU->LoTail, 0xDEADF00D);
}

TEST_F(CPUTest, InitialConditionsAfterBootWithStringOut) {
    boot_with_string_out_to_link(); // Wdesc will be different as it's based on bootlen
    interpret_until(0x800000FA); // end of typical boot, before ajw
    myCPU->DumpRegs(LOGLEVEL_DEBUG);
    myCPU->DumpQueueRegs(LOGLEVEL_DEBUG);
    myCPU->DumpClockRegs(LOGLEVEL_DEBUG, myCPU->InstCycles + myCPU->MemCycles);

    EXPECT_EQ(myCPU->IPtr, 0x800000FA);        // vvvv string out setup is 0x8A bytes
    EXPECT_EQ(myCPU->Wdesc, (WordAlign(MemStart + 0xA6)) | 0x00000001); // low priority => 0x80000119
    EXPECT_EQ(myCPU->HiHead, NotProcess_p);
    EXPECT_EQ(myCPU->HiTail, 0xDEADF00D);
    EXPECT_EQ(myCPU->LoHead, NotProcess_p);
    EXPECT_EQ(myCPU->LoTail, 0xDEADF00D);
}

TEST_F(CPUTest, AdjustWorkspace) {
    boot_with_string_out_to_link();
    // with a workspace that's been been ajw'd, what's stored in the process queues, the adjusted wdesc, or
    // the original? It's the adjusted wdesc.
    interpret_until(0x800000FD); // just after the ajw 0x100
    myCPU->DumpRegs(LOGLEVEL_DEBUG);

    EXPECT_EQ(myCPU->Wdesc, (WordAlign(MemStart + 0xA6 + 0x400)) | 0x00000001); // low priority => 0x80000519
}

TEST_F(CPUTest, LinkIODeschedulesToLinkLeavingIdle) {
    boot_with_string_out_to_link();
    WORD32 processWdesc = 0x80000519; // After the boot and ajw.
    interpret_until(0x80000109); // just before the out
    // Ensure we can sense the stored IPtr later...
	myMemory->setWord((processWdesc & WordMask) - 4, NotProcess_p);
    //
    EXPECT_EQ(IS_FLAG_SET(EmulatorState_DescheduleRequired), false);
    EXPECT_EQ(IS_FLAG_SET(EmulatorState_LinkDescheduleRequired), false);
    EXPECT_EQ(myLinks[0]->m_send_registers.m_workspace_pointer, NotProcess_p); // The link has no workspace yet
    EXPECT_EQ(myCPU->Wdesc, processWdesc); // it'll soon be IdleProcess.p

    interpret_until(0x8000010A); // just after the out (the ldc 42)
    myCPU->DumpRegs(LOGLEVEL_DEBUG);
    myCPU->DumpQueueRegs(LOGLEVEL_DEBUG);
	EXPECT_EQ(myMemory->getWord(W_IPTR(processWdesc)), 0x8000010A); // The IPtr is stored in the workspace

    EXPECT_EQ(myCPU->IPtr, 0x8000010A);
    EXPECT_EQ(IS_FLAG_SET(EmulatorState_DescheduleRequired), false); // This is set by out, and cleared at the end of
    // the deschedule code at the end of interpret(), so you can't sense it directly, only its effects.
    EXPECT_EQ(IS_FLAG_SET(EmulatorState_LinkDescheduleRequired), false); // Same, but it has been set during interpret.
    EXPECT_EQ(myCPU->Wdesc, IdleProcess_p);
    EXPECT_EQ(myCPU->HiHead, NotProcess_p); // It's a low priority process.
    EXPECT_EQ(myCPU->HiTail, 0xDEADF00D);
    EXPECT_EQ(myCPU->LoHead, NotProcess_p); // The process is not on a list, and the CPU is idle. It's with the link.
    EXPECT_EQ(myCPU->LoTail, 0xDEADF00D);
    // Does the W_LINK of the process get set to NotProcess_p when the process is with the link?
    EXPECT_EQ(myLinks[0]->m_send_registers.m_workspace_pointer, processWdesc); // The link has the workspace.
}

TEST_F(CPUTest, MemoryIODeschedulesToQueueLeavingIdle) {
    boot_with_string_out_to_memory();
    WORD32 processWdesc = 0x8000051D; // After the boot and ajw.
    interpret_until(0x8000010C); // just before the out
    logDebug("^^^ just before the out vvv prepping the IPtr save area for later test");
    // Ensure we can sense the stored IPtr later...
    myMemory->setWord((processWdesc & WordMask) - 4, NotProcess_p);
    //
    EXPECT_EQ(IS_FLAG_SET(EmulatorState_DescheduleRequired), false);
    EXPECT_EQ(myLinks[0]->m_send_registers.m_workspace_pointer, NotProcess_p); // The link has no workspace
    EXPECT_EQ(myCPU->Wdesc, processWdesc); // it'll soon be IdleProcess.p
    logDebug("vvvv running the out");
    interpret_until(0x8000010D); // just after the out (at the ldc 42)
    myCPU->DumpRegs(LOGLEVEL_DEBUG);
    myCPU->DumpQueueRegs(LOGLEVEL_DEBUG);
    EXPECT_EQ(myMemory->getWord(W_IPTR(processWdesc)), 0x8000010D); // The IPtr is stored in the workspace
    EXPECT_EQ(myMemory->getWord(W_POINTER(processWdesc)), 0x8000010F); // The buffer addr is stored in the workspace
    EXPECT_EQ(myMemory->getWord(0x8000011B), processWdesc); // The channel holds the workspace
    EXPECT_EQ(myCPU->IPtr, 0x8000010D); // But is it? The CPU should be idle as there's only this one process, and it's queued.
    EXPECT_EQ(IS_FLAG_SET(EmulatorState_DescheduleRequired), false); // This is set by out, and cleared at the end of
    // the deschedule code at the end of interpret(), so you can't sense it directly, only its effects.
    EXPECT_EQ(myCPU->HiHead, NotProcess_p); // It's a low priority process, so this is as setup leaves it.
    EXPECT_EQ(myCPU->HiTail, 0xDEADF00D); // As _emulate_prologue initialises it.
    EXPECT_EQ(myCPU->LoHead, processWdesc); // The process is placed at the sole entry in this list, and the CPU is idle.
    EXPECT_EQ(myCPU->LoTail, processWdesc);
    EXPECT_EQ(myMemory->getWord(W_LINK(processWdesc)), NotProcess_p); // No next process after this.
    EXPECT_EQ(myCPU->Wdesc, IdleProcess_p);
    EXPECT_EQ(myLinks[0]->m_send_registers.m_workspace_pointer, NotProcess_p); // The link still has no workspace
}


// TODO tests...
// Deschedule requested when idle?
//
// Out to memory channel, but the in reaches the rendezvous first.
//
// Two processes, one is already on the low priority list, and the second does a memory channel out, and is appended
//   to the end of the list. (We have a test MemoryIODeschedulesToQueueLeavingIdle that tests that the out process is
//   set as the sole member of the previously-empty list. Need to check the previously-not-empty case)
//
// High priority processes are not descheduled on j after clock expiry
// High priority processes are not descheduled on lend after clock expiry
// Link outbyte
// Link outword
// Link in
// Link direction bug: an in instruction with an out link address -> it's an out.
// Link direction bug: an out instruction with an in link address -> it's an in.
// Link direction bug: an outbyte instruction with an in link address -> it's an in.
// Link direction bug: an outword instruction with an in link address -> it's an in.
