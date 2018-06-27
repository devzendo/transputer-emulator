//------------------------------------------------------------------------------
//
// File        : cpu.h
// Description : CPU emulator
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 14/07/2005
//
// (C) 2005-2018 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _CPU_H
#define _CPU_H

#include "types.h"
#include "memory.h"
#include "link.h"
#include "linkfactory.h"

class CPU {
	public:
		// 2-phase CTOR since there's only one global CPU
		CPU();
		bool initialise(Memory *memory, LinkFactory *linkFactory); 
		void emulate(void);
		~CPU();
	private:
		// Dynamically allocated memory
		Memory *myMemory;
		Link *myLinks[4];
		// All registers
		WORD32 IPtr, Wdesc;
		WORD32 Areg, Breg, Creg, Oreg; // Integer register evaluation stack
		WORD32 OldOreg; // Holds valid Oreg for bad instruction trap
		WORD32 ScheduleWdesc; // NULL or Wdesc to schedule after interpretation
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
		BYTE CurrInstruction; // Currently fetched byte during instruction decode
		WORD32 Instruction,InstCycles,MemCycles; // Opcode storage, cycle counters
		WORD32 InstructionStartIPtr; // Start of instruction, for disassembly
		// Bootstrap storage
		BYTE bootLen;
		// Monitor usage
		WORD32 CurrDataAddress;
		WORD32 CurrDataLen;
		WORD32 CurrDisasmAddress;
		WORD32 CurrDisasmLen;

		// Internal methods:
		inline void DROP(void);
		inline WORD32 POP(void);
		inline void PUSH(WORD32 x);

		void DumpRegs(int logLevel);
		void DumpQueueRegs(int logLevel);
		void DumpClockRegs(int logLevel, WORD32 instCycles);

		inline void boot(void);
		void disassembleCurrInstruction(int logLevel);
		WORD32 disassembleRange(WORD32 addr, WORD32 maxlen);
		inline void interpret(void);
		inline bool monitor(void);

};

#endif // _CPU_H

