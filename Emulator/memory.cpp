//------------------------------------------------------------------------------
//
// File        : memory.cpp
// Description : memory subsystem
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 29/06/2005
//
// (C) 2005-2019 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <cstdlib>
#include <cctype>
#include <cstdio>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include "types.h"
#include "constants.h"
#include "memloc.h"
#include "memory.h"
#include "flags.h"
#include "log.h"

Memory::Memory() {
	logDebug("Memory CTOR");
	resetMemory();
}

void Memory::resetMemory() {
	myMemory = NULL;
	myReadOnlyMemory = NULL;
	myReadOnlyMemorySize = 0;
	mySize = 0;
	myMemEnd = InternalMemStart;
	myHighestAccess = InternalMemStart;
	myCurrentCycles = 0;
}

bool Memory::initialise(long initialRAMSize, const char *romFile) {
	myMemory = (BYTE *)calloc(initialRAMSize, 1);
	if (myMemory == NULL) {
		logFatal("Failed to allocate memory");
		return false;
	}
	/*for (int i=0; i<initialRAMSize; i++) {
		myMemory[i] = 0xAA;
	}*/
	myMemEnd = InternalMemStart + initialRAMSize;
	mySize = initialRAMSize;

	if (romFile) {
		myROMPresent = true;
		if (!loadROMFile(romFile)) {
			return false;
		}
	} else {
		myROMPresent = false;
		myReadOnlyMemory = NULL;
		myReadOnlyMemorySize = 0;
	}
	return true;
}

// See CWG, p74
bool Memory::loadROMFile(const char *fileName) {
	struct stat st;
	if (stat(fileName, &st) == -1) {
		logFatalF("Could not obtain details of ROM file %s: %s", fileName, strerror(errno));
		return false;
	}

	const off_t romSize = st.st_size;
	const WORD32 romSize32 = (WORD32) romSize;
	myReadOnlyMemorySize = romSize32;
	myReadOnlyMemory = (BYTE *)calloc(myReadOnlyMemorySize, 1);
	if (myReadOnlyMemory == NULL) {
		logFatal("Failed to allocate Read-Only memory");
		return false;
	}

	myROMStart = MaxINT - romSize32 + 1;
	logDebugF("ROM (size %d bytes) will be loaded from %08X to %08X", romSize32, myROMStart, MaxINT);
	if (!isLegalMemory(myROMStart)) {
		logFatalF("Boot from ROM cannot load at bad address %08X", myROMStart);
		return false;
	}

	const int readFD = open(fileName, O_RDONLY);
	if (readFD == -1) {
		logFatalF("Could not open ROM file %s: %s", fileName, strerror(errno));
		return false;
	}

	const ssize_t readBytes = read(readFD, myReadOnlyMemory, myReadOnlyMemorySize);
	if (readBytes != myReadOnlyMemorySize) {
		logFatalF("Tried to load %08X bytes from ROM file %s, but only read %08X", readBytes, fileName, romSize32);
		return false;
	}

	close(readFD);
	return true;
}

Memory::~Memory() {
	logDebugF("Memory DTOR - this is 0x%lx, Memory is 0x%lx, ROM is 0x%lx", this, myMemory, myReadOnlyMemory);
	if (myMemory != NULL) {
		logDebug("Memory is not NULL - freeing");
		free(myMemory);
	}
	if (myReadOnlyMemory != NULL) {
		logDebug("Read-Only Memory is not NULL - freeing");
		free(myReadOnlyMemory);
	}
	if (myMemory != NULL || myReadOnlyMemory != NULL) {
		resetMemory();
	}
}

int Memory::getMemEnd() {
	return myMemEnd;
}

int Memory::getMemSize() {
	return mySize;
}
int Memory::getHighestAccess() {
	return myHighestAccess;
}

// TODO fix external memory access taking longer than internal access - this
// isn't a precise emulation of memory speed.
BYTE Memory::getByte(WORD32 addr) {
	BYTE b;
	if (addr >= InternalMemStart && addr <= myMemEnd) {
		myCurrentCycles += 1;
		if (addr > myHighestAccess) {
			myHighestAccess = addr;
		}
		b = myMemory[addr - InternalMemStart];
		if ((flags & DebugFlags_MemAccessDebugLevel) != MemAccessDebug_No) {
			logDebugF("R 1 [%08X]=%02X (%c)", addr, b, isprint(b) ? b : '?');
		}
	} else if (myROMPresent && addr >= myROMStart && addr <= MaxINT) {
		myCurrentCycles += 1;
		// not tracking highest ROM access here
		b = myReadOnlyMemory[addr - myROMStart];
		if ((flags & DebugFlags_MemAccessDebugLevel) != MemAccessDebug_No) {
			logDebugF("R 1 [%08X]=%02X (%c)", addr, b, isprint(b) ? b : '?');
		}
	} else {
		if (IS_FLAG_SET(DebugFlags_TerminateOnMemViol)) {
			logFatalF("Memory violation reading byte from %08X", addr);
			SET_FLAGS(EmulatorState_Terminate);
		} else {
			logErrorF("Memory violation reading byte from %08X", addr);
		}
		b = 0x00;
	}
	return b;
}

BYTE Memory::getInstruction(WORD32 addr) {
	BYTE b;
	if (addr >= InternalMemStart && addr <= myMemEnd) {
		myCurrentCycles += 1;
		if (addr > myHighestAccess) {
			myHighestAccess = addr;
		}
		b = myMemory[addr - InternalMemStart];
		if ((flags & DebugFlags_MemAccessDebugLevel) == MemAccessDebug_Full) {
			logDebugF("I 1 [%08X]=%02X", addr, b);
		}
	} else if (myROMPresent && addr >= myROMStart && addr <= MaxINT) {
		myCurrentCycles += 1;
		// not tracking highest ROM access here
		b = myReadOnlyMemory[addr - myROMStart];
		if ((flags & DebugFlags_MemAccessDebugLevel) == MemAccessDebug_Full) {
			logDebugF("I 1 [%08X]=%02X", addr, b);
		}
	} else {
		if (IS_FLAG_SET(DebugFlags_TerminateOnMemViol)) {
			logFatalF("Memory violation reading instruction from %08X", addr);
			SET_FLAGS(EmulatorState_Terminate);
		} else {
			logErrorF("Memory violation reading instruction from %08X", addr);
		}
		b = 0x00;
	}
	return b;
}

void Memory::setByte(WORD32 addr, BYTE value) {
	if (addr >= InternalMemStart && addr <= myMemEnd) {
		myCurrentCycles += 1;
		if (addr > myHighestAccess) {
			myHighestAccess = addr;
		}
		myMemory[addr - InternalMemStart] = value;
		if ((flags & DebugFlags_MemAccessDebugLevel) != MemAccessDebug_No) {
			logDebugF("W 1 [%08X]=%02X", addr, value);
		}
	} else if (myROMPresent && addr >= myROMStart && addr <= MaxINT) {
		if (IS_FLAG_SET(DebugFlags_TerminateOnMemViol)) {
			logFatalF("Memory violation writing byte to ROM %08X", addr);
			SET_FLAGS(EmulatorState_Terminate);
		} else {
			logErrorF("Memory violation writing byte to ROM %08X", addr);
		}
	} else {
		if (IS_FLAG_SET(DebugFlags_TerminateOnMemViol)) {
			logFatalF("Memory violation writing byte to %08X", addr);
			SET_FLAGS(EmulatorState_Terminate);
		} else {
			logErrorF("Memory violation writing byte to %08X", addr);
		}
	}
}

WORD32 Memory::getWord(WORD32 addr) {
	BYTE *b;
	WORD32 w;
	if (addr >= InternalMemStart && addr <= myMemEnd) {
		myCurrentCycles += 1;
		if (addr > myHighestAccess) {
			myHighestAccess = addr;
		}
		b = myMemory + (addr - InternalMemStart);
		// Irrespective of the emulator hosts's endianness, words are
		// always stored in memory in little-endian form, as on a real
		// Transputer. LSB first MSB last
		w = (b[3] << 24) | (b[2] << 16) | (b[1] << 8) | b[0];
		if ((flags & DebugFlags_MemAccessDebugLevel) != MemAccessDebug_No) {
			logDebugF("R 4 [%08X]=%08X", addr, w);
		}
	} else if (myROMPresent && addr >= myROMStart && addr <= MaxINT) {
		myCurrentCycles += 1;
		// not tracking highest ROM read
		b = myReadOnlyMemory + (addr - myROMStart);
		// Irrespective of the emulator hosts's endianness, words are
		// always stored in memory in little-endian form, as on a real
		// Transputer. LSB first MSB last
		w = (b[3] << 24) | (b[2] << 16) | (b[1] << 8) | b[0];
		if ((flags & DebugFlags_MemAccessDebugLevel) != MemAccessDebug_No) {
			logDebugF("R 4 [%08X]=%08X", addr, w);
		}
	} else {
		if (IS_FLAG_SET(DebugFlags_TerminateOnMemViol)) {
			logFatalF("Memory violation reading word from %08X", addr);
			SET_FLAGS(EmulatorState_Terminate);
		} else {
			logErrorF("Memory violation reading word from %08X", addr);
		}
		w = 0xC0DEDBAD;
	}
	return w;
}

void Memory::setWord(WORD32 addr, WORD32 value) {
	BYTE *b;
	if (addr >= InternalMemStart && addr <= myMemEnd) {
		myCurrentCycles += 1;
		if (addr > myHighestAccess) {
			myHighestAccess = addr;
		}
		b = myMemory + (addr - InternalMemStart);
		// Irrespective of the emulator hosts's endianness, words are
		// always stored in memory in little-endian form, as on a real
		// Transputer. LSB first MSB last
		b[0] = (value & 0x000000ff);
		b[1] = (value & 0x0000ff00) >> 8;
		b[2] = (value & 0x00ff0000) >> 16;
		b[3] = (value & 0xff000000) >> 24;
		if ((flags & DebugFlags_MemAccessDebugLevel) != MemAccessDebug_No) {
			logDebugF("W 4 [%08X]=%08X", addr, value);
		}
	} else if (myROMPresent && addr >= myROMStart && addr <= MaxINT) {
		if (IS_FLAG_SET(DebugFlags_TerminateOnMemViol)) {
			logFatalF("Memory violation writing word to ROM %08X", addr);
			SET_FLAGS(EmulatorState_Terminate);
		} else {
			logErrorF("Memory violation writing word to ROM %08X", addr);
		}
	} else {
		if (IS_FLAG_SET(DebugFlags_TerminateOnMemViol)) {
			logFatalF("Memory violation writing word to %08X", addr);
			SET_FLAGS(EmulatorState_Terminate);
		} else {
			logErrorF("Memory violation writing word to %08X", addr);
		}
	}
}

int Memory::getCurrentCyclesAndReset() {
	int t = myCurrentCycles;
	myCurrentCycles = 0;
	return t;
}

// Calculate the number of words in a block.
// This is used when determining the correct number of words to be used
// when calculating instruction cycles for block operations, i.e. move, in, and
// out.
// The number of words in a block is defined as (TTS p15):
// Part words are counted as full words. If the block is not word aligned
// the number of words is increased to include the part words ar either end of
// the block.
static int wordsInBlock(WORD32 len, WORD32 addr) {
	return (len == 0) ?
		0 :
		(((addr + len + 3) & WordMask) - (addr & WordMask)) / 4;
}


void Memory::blockCopy(WORD32 len, WORD32 srcAddr, WORD32 destAddr) {
	int i, j;
	// The speed of the copy is based on 2w+c where w is the number of
	// words and part-words involved in the copy. The memory speed of this
	// emulator is based on all memory being internal. c is the number of
	// cycles used by the operation, e.g. input, output or move.
	bool ok = true;
	WORD32 addr;
	// TODO check this algo...
	// How long will the read take?
	for (i = 0, j = wordsInBlock(len, srcAddr); i < j; i++) {
		addr = srcAddr + (i << 2);
		if (addr >= InternalMemStart && addr <= myMemEnd) {
			myCurrentCycles += 1;
		}
	}
	// How long will the write take?
	for (i = 0, j = wordsInBlock(len, destAddr); i < j; i++) {
		addr = destAddr + (i << 2);
		if (addr >= InternalMemStart && addr <= myMemEnd) {
			myCurrentCycles += 1;
		}
	}
	// Do copy in bytes
	// TODO optimise for best native performance later
	WORD32 sA, dA;
	BYTE b;
	for (i = 0, sA = srcAddr, dA = destAddr; i < len; i++, sA++, dA++) {
		// Read from source...
		if (sA >= InternalMemStart && sA <= myMemEnd) {
			if (sA > myHighestAccess) {
				myHighestAccess = sA;
			}
			b = myMemory[sA - InternalMemStart];
			if ((flags & DebugFlags_MemAccessDebugLevel) != MemAccessDebug_No) {
				logDebugF("R 1 [%08X]=%02X", sA, b);
			}
		} else if (myROMPresent && sA >= myROMStart && sA <= MaxINT) {
			// not tracking highest ROM access
			b = myReadOnlyMemory[sA - myROMStart];
			if ((flags & DebugFlags_MemAccessDebugLevel) != MemAccessDebug_No) {
				logDebugF("R 1 [%08X]=%02X", sA, b);
			}
		} else {
			if (IS_FLAG_SET(DebugFlags_TerminateOnMemViol)) {
				logFatalF("Memory violation reading block from %08X", sA);
				SET_FLAGS(EmulatorState_Terminate);
			} else {
				logErrorF("Memory violation reading block from %08X", sA);
			}
			return;
		}
		// Write to destination
		if (dA >= InternalMemStart && dA <= myMemEnd) {
			if (dA > myHighestAccess) {
				myHighestAccess = dA;
			}
			myMemory[dA - InternalMemStart] = b;
			if ((flags & DebugFlags_MemAccessDebugLevel) != MemAccessDebug_No) {
				logDebugF("W 1 [%08X]=%02X", dA, b);
			}
		} else if (myROMPresent && dA >= myROMStart && dA <= MaxINT) {
			if (IS_FLAG_SET(DebugFlags_TerminateOnMemViol)) {
				logFatalF("Memory violation writing block at ROM %08X", dA);
				SET_FLAGS(EmulatorState_Terminate);
			} else {
				logErrorF("Memory violation writing block at ROM %08X", dA);
			}
			return;
		} else {
			if (IS_FLAG_SET(DebugFlags_TerminateOnMemViol)) {
				logFatalF("Memory violation writing block at %08X", dA);
				SET_FLAGS(EmulatorState_Terminate);
			} else {
				logErrorF("Memory violation writing block at %08X", dA);
			}
			return;
		}
	}
}

bool Memory::isLegalMemory(WORD32 addr) {
	return (addr >= InternalMemStart && addr <= myMemEnd) ||
			(myROMPresent && addr >= myROMStart && addr <= MaxINT);
}

static char hexdigs[]="0123456789abcdef";

void Memory::hexDump(WORD32 addr, WORD32 len) {
char line[80];
WORD32 i;
WORD32 offset=addr;
SWORD32 left=len;
SWORD32 upto16, x;
BYTE b;
	while (left > 0) {
		for (i = 0; i < 78; i++) {
			line[i] = ' ';
		}
		line[9] = line[59]='|';
		line[77] = '\0';
		sprintf(line, "%08X", offset);
		line[8] = ' ';
		upto16 = (left > 16) ? 16 : left;
		for (x = 0; x < upto16; x++) {
			WORD32 byteAddr = offset + x;
			if (isLegalMemory(byteAddr)) {

				// Repeating some of getByte's internals here without the memory access diagnostics
				if (byteAddr >= InternalMemStart && byteAddr <= myMemEnd) {
					b = myMemory[byteAddr - InternalMemStart];
				} else if (myROMPresent && byteAddr >= myROMStart && byteAddr <= MaxINT) {
					b = myReadOnlyMemory[byteAddr - myROMStart];
				}

				line[11 + (3 * x)] = hexdigs[(b & 0xf0) >> 4];
				line[12 + (3 * x)] = hexdigs[b & 0x0f];
				line[61 + x] = isprint((char)b) ? ((char)b) : '.';
			} else {
				line[11 + (3 * x)] = '-';
				line[12 + (3 * x)] = '-';
				line[61 + x] = '-';
			}
		}
		logInfo(line);
		offset += upto16;
		left -= upto16;
	}
}

// Precondition: lenInBytes is a multiple of the word size, 4 bytes
void Memory::hexDumpWords(WORD32 addr, WORD32 lenInBytes) {
char line[80];
WORD32 i;
WORD32 offset=addr;
SWORD32 leftBytes=lenInBytes;
SWORD32 upto4Words, x;
BYTE *b;
WORD32 w;
/*        1         2         3         4         5         6         7
01234567890123456789012345678901234567890123456789012345678901234567890123456789
00000000 | 01234567 01234567 01234567 01234567 | .... .... .... ....
 */
	while (leftBytes > 0) {
		for (i = 0; i < 78; i++) {
			line[i] = ' ';
		}
		line[9] = line[47] = '|';
		line[68] = '\0';
		sprintf(line, "%08X", offset);
		line[8] = ' ';
		upto4Words = (leftBytes > 16) ? 4 : (leftBytes >> 2);
		for (x = 0; x < upto4Words; x++) {
			WORD32 wordAddr = offset + (x << 2);
			if (isLegalMemory(wordAddr)) {

				// Repeating some of getWord's internals here without the memory access diagnostics.
				// Irrespective of the emulator hosts's endianness, words are
				// always stored in memory in little-endian form, as on a real
				// Transputer. LSB first MSB last.
				if (wordAddr >= InternalMemStart && wordAddr <= myMemEnd) {
 					b = myMemory + (wordAddr - InternalMemStart);
				} else if (myROMPresent && wordAddr >= myROMStart && wordAddr <= MaxINT) {
 					b = myReadOnlyMemory + (wordAddr - myROMStart);
				} // must be one of those branches, since isLegalMemory is true.

				BYTE b0 = b[0];
				BYTE b1 = b[1];
				BYTE b2 = b[2];
				BYTE b3 = b[3];
				w = (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
				sprintf(line + 11 + (9 * x), "%08X", w);
				line[49 + (x * 5)] = isprint((char)b0) ? ((char)b0) : '.';
				line[50 + (x * 5)] = isprint((char)b1) ? ((char)b1) : '.';
				line[51 + (x * 5)] = isprint((char)b2) ? ((char)b2) : '.';
				line[52 + (x * 5)] = isprint((char)b3) ? ((char)b3) : '.';
			} else {
				sprintf(line + 11 + (9 * x), "--------");
				sprintf(line + 49 + (5 * x), "---- ");
			}
			line[19 + (9 * x)] = ' ';
		}
		logInfo(line);
		offset += 16;
		leftBytes -= 16;
	}
}


