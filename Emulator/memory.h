//------------------------------------------------------------------------------
//
// File        : memory.h
// Description : memory subsystem
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 29/06/2005
// Revision    : $Revision $
//
// (C) 2005-2018 Matt J. Gumbley
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
		bool initialise(long initialSize); 
		~Memory();
		int getMemEnd();
		int getMemSize();
		int getHighestAccess();
		BYTE getByte(WORD32 addr);
		BYTE getInstruction(WORD32 addr);
		void setByte(WORD32 addr, BYTE value);
		WORD32 getWord(WORD32 addr);
		void setWord(WORD32 addr, WORD32 value);
		int getCurrentCyclesAndReset();
		void blockCopy(WORD32 len, WORD32 srcAddr, WORD32 destAddr);
		bool isLegalMemory(WORD32 addr);
		// Used by the monitor
		void hexDump(WORD32 addr, WORD32 len);
		void hexDumpWords(WORD32 addr, WORD32 len);
	private:
		BYTE *myMemory;
		int mySize;
		int myMemEnd;
		int myHighestAccess;
		//=(InternalMemStart + MemSize);
		void resetMemory();
		int myCurrentCycles;
};

#endif // _MEMORY_H

