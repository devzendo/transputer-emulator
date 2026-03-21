//------------------------------------------------------------------------------
//
// File        : memory.h
// Description : memory subsystem
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 29/06/2005
//
// (C) 2005-2026 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"
#include "symbol.h"

class Memory {
	public:
		// 2-phase CTOR since there's only one global Memory
		Memory();
		bool initialise(long initialRAMSize);
#ifdef DESKTOP
		bool initialiseROMFileAndSymbolTable(const char *romFileName, SymbolTable *symbolTable);
#endif
		~Memory();
		WORD32 getMemEnd() const;
		long getMemSize() const;
		WORD32 getHighestAccess() const;
		BYTE8 getByte(WORD32 addr);
		BYTE8 getInstruction(WORD32 addr);
		void setByte(WORD32 addr, BYTE8 value);
		WORD32 getWord(WORD32 addr);
		void setWord(WORD32 addr, WORD32 value);
		int getCurrentCyclesAndReset();
		void blockCopy(WORD32 len, WORD32 srcAddr, WORD32 destAddr);
		bool isLegalMemory(WORD32 addr) const;
		// Used by the monitor
		void hexDump(WORD32 addr, WORD32 len);
		void hexDumpWords(WORD32 addr, WORD32 lenInBytes);
	private:
#ifdef DESKTOP
		SymbolTable *mySymbolTable{};
#endif
		bool loadROMFile(const char *romFileName);
		BYTE8 *myMemory{};
		long mySize{};
		WORD32 myMemEnd{};
		WORD32 myHighestAccess{};
		//=(InternalMemStart + MemSize);
		void resetMemory();
		int myCurrentCycles{};
		// ROM (if present) extends from myROMStart, until MaxINT - it is loaded at the end of memory, so that the
		// 2-byte jump at ResetCode is present.
		bool myROMPresent{};
		WORD32 myROMStart{};
		BYTE8 *myReadOnlyMemory{};
		size_t myReadOnlyMemorySize{};
};

#endif // MEMORY_H

