//------------------------------------------------------------------------------
//
// File        : memory.h
// Description : memory subsystem
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 29/06/2005
//
// (C) 2005-2020 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _MEMORY_H
#define _MEMORY_H

#include "types.h"

class Memory {
	public:
		// 2-phase CTOR since there's only one global Memory
		Memory();
		bool initialise(long initialRAMSize, const char *romFileName);
		~Memory();
		WORD32 getMemEnd();
		int getMemSize();
		WORD32 getHighestAccess();
		BYTE8 getByte(WORD32 addr);
		BYTE8 getInstruction(WORD32 addr);
		void setByte(WORD32 addr, BYTE8 value);
		WORD32 getWord(WORD32 addr);
		void setWord(WORD32 addr, WORD32 value);
		int getCurrentCyclesAndReset();
		void blockCopy(WORD32 len, WORD32 srcAddr, WORD32 destAddr);
		bool isLegalMemory(WORD32 addr);
		// Used by the monitor
		void hexDump(WORD32 addr, WORD32 len);
		void hexDumpWords(WORD32 addr, WORD32 lenInBytes);
	private:
		bool loadROMFile(const char *romFileName);
		BYTE8 *myMemory;
		int mySize;
		WORD32 myMemEnd;
		WORD32 myHighestAccess;
		//=(InternalMemStart + MemSize);
		void resetMemory();
		int myCurrentCycles;
		// ROM (if present) extends from myROMStart, until MaxINT - it is loaded at the end of memory, so that the
		// 2-byte jump at ResetCode is present.
		bool myROMPresent;
		WORD32 myROMStart;
		BYTE8 *myReadOnlyMemory;
		size_t myReadOnlyMemorySize;

};

#endif // _MEMORY_H

