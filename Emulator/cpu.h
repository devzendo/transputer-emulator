//------------------------------------------------------------------------------
//
// File        : cpu.h
// Description : CPU emulator
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 14/07/2005
//
// (C) 2005-2026 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _CPU_H
#define _CPU_H

#include <set>
#include <deque>

#include "constants.h"
#include "types.h"
#include "memory.h"
#include "link.h"
#include "linkfactory.h"
#include "platformdetection.h"
#include "symbol.h"
#include "boot.h"

// So that the tests can have friend access to CPU internals. And the tests only run on DESKTOP builds.
#ifdef DESKTOP
#include "gtest/gtest_prod.h"
#endif

// Special offsets below the workspace pointer, or other pointer:
inline WORD32 Wdesc_Priority(WORD32 Wdesc) {
	return Wdesc & 0x01;
}

inline WORD32 Wdesc_HiPriority(WORD32 Wdesc) {
	return (Wdesc & 0x01) == 0;
}

inline WORD32 Wdesc_WPtr(WORD32 Wdesc) {
	return Wdesc & WordMask;
}

// TODO: Use a structure/union for this?
//
// Temporary register used by outbyte, outword, postnormsn.
// Also holds address of selected
// process during execution of an alt
inline WORD32 W_TEMP(WORD32 x) {
	return Wdesc_WPtr(x);
}

// If the process has been descheduled, this holds the IPtr
// of the descheduled process.
inline WORD32 W_IPTR(WORD32 x) {
	return Wdesc_WPtr(x) - 4;
}

// If the process is on a scheduling list has been
// descheduled then rescheduled, this is the address
// of the next process on the list's workspace.
inline WORD32 W_LINK(WORD32 x) {
	return Wdesc_WPtr(x) - 8;
}

// During communication deschedule, or during an alt,
// this holds the address of the channel. During alt, it holds
// the alternation state. These two are synonymous.
inline WORD32 W_POINTER(WORD32 x) {
	return Wdesc_WPtr(x) - 12;
}
inline WORD32 W_ALTSTATE(WORD32 x) {
	return Wdesc_WPtr(x) - 12;
}

// The address of the next process's workspace, if on a timer
// list, or state of time selection in an alternation
// involving timer guards.
inline WORD32 W_TLINK(WORD32 x) {
	return Wdesc_WPtr(x) - 16;
}

// The time that a process on a timer list is waiting for.
inline WORD32 W_TIME(WORD32 x) {
	return Wdesc_WPtr(x) - 20;
}

inline WORD64 MakeWORD64(WORD32 MSW, WORD32 LSW) {
	return (((WORD64)MSW) << BitsPerWord) | (((WORD64)LSW) & 0x00000000FFFFFFFF);
}

inline WORD32 WordAlign(WORD32 addr) {
	return (addr + 3) & WordMask;
}

class CPU {
	friend class CPUTest;

	public:
		// 2-phase CTOR since there's only one global CPU
		CPU();
		// Initialise either with the link factory, which will create the links, or with the directly created links,
		// for example, one of a pair of InMemoryLinks.
		bool initialise(Memory *memory, Link *links[4]);
		bool initialise(Memory *memory, LinkFactory *linkFactory);
#ifdef DESKTOP
		void initialiseSymbolTable(SymbolTable *symbolTable);
		void seteForthStackAddresses(WORD32 SPP, WORD32 RPP);
#endif
		void addBreakpoint(WORD32 breakpointAddress);
		void removeBreakpoint(WORD32 breakpointAddress);
		void emulate(const bool bootFromROM);
		// emulate is split into a prologue, a middle (loop), and an epilogue. Prologue and epilogue are also called by
		// testcpu, with the loop being replaced in tests by the interpretations of each test.
		void _emulate_prologue(const bool bootFromROM);
		void _emulate_loop();
		void _emulate_epilogue();
		void start();
		~CPU();
	private:
		// Dynamically allocated memory
		Memory *myMemory;
		Link *myLinks[4];
		Boot *myBoot;
		// All registers
		WORD32 IPtr, Wdesc; // Wdesc holds workspace pointer and priority - use Wdesc_Ptr/Wdesc_Priority to access parts
		// There is no separate Wptr/Priority register
		WORD32 Areg, Breg, Creg, Oreg; // Integer register evaluation stack
		WORD32 OldOreg; // Holds valid Oreg for bad instruction trap
		WORD32 ScheduleWdesc; // NULL or Wdesc to schedule after interpretation
		// TODO move the schedulables[9] here and use schedulables[0] to schedule the interpreted process
		REAL64 FAreg, FBreg, FCreg; // Floating point evaluation stack
		WORD32 HiHead, HiTail, LoHead, LoTail; // Process queue pointers
		WORD32 HiTimerHead, LoTimerHead; // Timer queue head pointers
		WORD32 HiTimeout, LoTimeout; // Time at which next timer event occurs
		WORD32 InterpFlagSet; // What to turn on before interpreting
		// Timing variables
		// There is a 50ns clock cycle on a 20MHz Transputer, all
		// oher timing is derived from this. Timeslice period is 
		// 20480 50ns clock cycles. (5120 cycles @ 5MHz); ~ 1ms
		WORD32 CycleCount; // Total processor clock ticks
		WORD32 CycleCountSinceReset; // Total processor clock ticks  since last sttimer
		WORD32 HiClock; // High priority timer ticks every 1us
		WORD32 LoClock; // Low priority timer ticks every 64us
		WORD32 LoClockLastQuantumExpiry; // When the last expiry occurred
		WORD32 QuantumRemaining; // How long this low-priority process has left
		// Interpretation decode
		BYTE8 CurrInstruction; // Currently fetched byte during instruction decode
		WORD32 Instruction,InstCycles,MemCycles; // Opcode storage, cycle counters
		WORD32 InstructionStartIPtr; // Start of instruction, for disassembly
		// Bootstrap storage
		BYTE8 bootLen;
		// Monitor usage
		WORD32 CurrDataAddress;
		WORD32 CurrDataLen;
		WORD32 CurrDisasmAddress;
		WORD32 CurrDisasmLen;
		WORD32 LastAjwInBytes;
		set<WORD32> BreakpointAddresses;
#ifdef DESKTOP
		bool myBootFromROM;
		SymbolTable *mySymbolTable;
		// eForth diagnostic; start of data and return stack
		WORD32 SPP;
		WORD32 RPP;
		deque<std::string> WordStack;
		std::string PossiblyColonWord;
		std::string CodeSymbol;

    FRIEND_TEST(CPUTest, InitialConditions);
    FRIEND_TEST(CPUTest, InitialConditionsAfterTypicalSetup);
    FRIEND_TEST(CPUTest, InitialConditionsAfterBootWithStringOut);
    FRIEND_TEST(CPUTest, AdjustWorkspace);
    FRIEND_TEST(CPUTest, LinkIODeschedulesToLinkLeavingIdle);
    FRIEND_TEST(CPUTest, MemoryIODeschedulesToQueueLeavingIdle);
    FRIEND_TEST(CPUTest, IdleDetection);

#endif

		// Internal methods:
		inline void DROP(void);
		inline WORD32 POP(void);
		inline void PUSH(WORD32 x);
		void interpret(void);
		inline void bootFromLink0(void);
		bool swapContextForBreakpointInstruction(void);
		inline bool monitor(void);

		void DumpRegs(int logLevel) const;
		void DumpQueueRegs(int logLevel) const;
		void DumpClockRegs(int logLevel, WORD32 instCycles) const;
		void disassembleCurrInstruction(int logLevel);
		WORD32 disassembleRange(WORD32 addr, WORD32 maxlen);
		void showBreakpointAddresses();
#ifdef DESKTOP
		void DumpeForthDiagnostics(int logLevel);
		inline void bootFromROMFile(const char *fileName);
#endif

};

#endif // _CPU_H

