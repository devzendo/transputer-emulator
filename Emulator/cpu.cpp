//------------------------------------------------------------------------------
//
// File        : cpu.cpp
// Description : CPU emulator
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 14/07/2005
//
// (C) 2005-2023 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <exception>
using namespace std;

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <string>

#include <iomanip>
#include <iostream>

#include "types.h"
#include "memloc.h"
#include "memory.h"
#include "log.h"
#include "cpu.h"
#include "constants.h"
#include "flags.h"
#include "link.h"
#include "linkfactory.h"
#include "opcodes.h"
#include "disasm.h"
#include "symbol.h"

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




WORD32 HighestSetBit(WORD32 Register) {
	int bitno;
	// Set the 0th bit as a sentinel
	Register |= 0x01;
	for (bitno = 31; bitno != -1; bitno--) {
		if (Register & 0x80000000) {
			return bitno;
		}
		else {
			Register <<= 1;
		}
	}
	logBug("HighestSetBit: sentinel bit not found");
	return 0;
}


inline WORD64 MakeWORD64(WORD32 MSW, WORD32 LSW) {
	return (((WORD64)MSW) << BitsPerWord) | (((WORD64)LSW) & 0x00000000FFFFFFFF);
}

inline WORD32 WordAlign(WORD32 addr) {
	return (addr + 3) & WordMask;
}




CPU::CPU() {
	logDebug("CPU CTOR");
}

bool CPU::initialise(Memory *memory, LinkFactory *linkFactory, SymbolTable *symbolTable) {
	int i;
	bool allLinksOK = true;
	mySymbolTable = symbolTable;
	myMemory = memory;
	for (i = 0; i < 4; i++) {
		if ((myLinks[i] = linkFactory->createLink(i)) == NULL) {
			logFatalF("Could not create link %d", i);
			allLinksOK = false;
		}
		try {
			myLinks[i]->initialise();
		} catch (exception &e) {
			logFatalF("Could not initialise link %d: %s", i, e.what());
			allLinksOK = false;
		}
	}

	if (!allLinksOK) {
		logFatal("Link setup failed");
		return false;
	}
	return true;
}

void CPU::addBreakpoint(const WORD32 breakpointAddress) {
	BreakpointAddresses.insert(breakpointAddress);
}

void CPU::removeBreakpoint(const WORD32 breakpointAddress) {
	if (BreakpointAddresses.erase(breakpointAddress) == 0) {
		logInfoF("Breakpoint not present: %08X", breakpointAddress);
	}
}

void CPU::seteForthStackAddresses(WORD32 aSPP, WORD32 aRPP) {
    SPP = aSPP;
    RPP = aRPP;
}

CPU::~CPU() {
	logDebug("CPU DTOR");
	for (int i = 0; i < 4; i++) {
		if (myLinks[i] != NULL) {
			delete myLinks[i];
			myLinks[i] = NULL;
		}
	}
}

void CPU::DumpRegs(int logLevel) {
	logFormat(logLevel, "%c%c%c%c%c%c A #%08X B #%08X C #%08X W #%08X",
		(Wdesc_HiPriority(Wdesc) ? 'H' : 'L'),
		(IS_FLAG_SET(EmulatorState_ErrorFlag) ? 'E' : '-'),
		(IS_FLAG_SET(EmulatorState_FErrorFlag) ? 'F' : '-'),
		(IS_FLAG_SET(EmulatorState_HaltOnError) ? 'H' : '-'),
		(IS_FLAG_SET(EmulatorState_DescheduleRequired) ? 'D' :
		(IS_FLAG_SET(EmulatorState_DeschedulePending) ? 'd' : '-')),
		(IS_FLAG_SET(EmulatorState_Interrupt) ? 'I' : '-'),
		Areg, Breg, Creg, Wdesc);
	bool aSymbol = mySymbolTable->addressExists(Areg);
	bool bSymbol = mySymbolTable->addressExists(Breg);
	bool cSymbol = mySymbolTable->addressExists(Creg);
	bool wSymbol = mySymbolTable->addressExists(Wdesc);
	if (aSymbol | bSymbol | cSymbol | wSymbol) {
		logFormat(logLevel, "       A %9s B %9s C %9s W %9s", 
			mySymbolTable->symbolOrEmptyString(Areg).c_str(), mySymbolTable->symbolOrEmptyString(Breg).c_str(),
			mySymbolTable->symbolOrEmptyString(Creg).c_str(), mySymbolTable->symbolOrEmptyString(Wdesc).c_str());
	}
	bool oSymbol = mySymbolTable->addressExists(Oreg);
	bool iSymbol = mySymbolTable->addressExists(IPtr);
	logFormat(logLevel,   "       O #%08X I #%08X", Oreg, IPtr);
	if (oSymbol | iSymbol) {
		logFormat(logLevel, "       O %9s I %9s",
			mySymbolTable->symbolOrEmptyString(Oreg).c_str(), mySymbolTable->symbolOrEmptyString(IPtr).c_str());
	}
}

void CPU::DumpQueueRegs(int logLevel) {
	logFormat(logLevel, "       Hf#%08X Hb#%08X Lf#%08X Lb#%08X",
		HiHead, HiTail, LoHead, LoTail);
}

void CPU::DumpClockRegs(int logLevel, WORD32 instCycles) {
	int qr;
	qr = LoClock - LoClockLastQuantumExpiry;
	if (qr >= MaxQuantum)
		qr = 0;
	else
		qr = MaxQuantum - qr;
	logFormat(logLevel, "       Hc#%08X Lc#%08X Qr#%08X C##%08X",
		HiClock, LoClock, qr, instCycles);
}

void CPU::DumpeForthDiagnostics(int logLevel) {
	// Shenanigans to build an idea of the currently nested word execution
	// based on colon words, code words, and words/sequences that have funky
	// pop action.
	if (mySymbolTable->addressExists(IPtr - 1)) {
		std::string symbol = mySymbolTable->getSymbolName(IPtr - 1);
		// Ignore jump labels that end in a digit
		char lastChar = symbol.at(symbol.size() - 1);
		if (!isdigit(lastChar)) {
			bool isCode = symbol == "BYE" || symbol == "QRX" ||
				symbol == "TXSTO" || symbol == "STOIO" ||
				symbol == "TDISASM" || symbol == "TERMINATE" ||
				symbol == "DOLIT" || symbol == "DOLST" ||
				symbol == "EXIT" || symbol == "EXECU" || 
				symbol == "DONXT" || symbol == "QBRAN" ||
				symbol == "BRAN" || symbol == "STORE" ||
				symbol == "AT" || symbol == "CSTOR" ||
				symbol == "CAT" || symbol == "RPAT" ||
				symbol == "RPSTO" || symbol == "RFROM" ||
				symbol == "RAT" || symbol == "TOR" ||
				symbol == "SPAT" || symbol == "SPSTO" ||
				symbol == "DROP" || symbol == "DUPP" ||
				symbol == "SWAP" || symbol == "OVER" ||
				symbol == "ZLESS" || symbol == "ANDD" ||
				symbol == "ORR" || symbol == "XORR" ||
				symbol == "UMPLUS";
			if (isCode) {
				CodeSymbol = symbol;
			}
			// DOUSE has a rapid double pop action :)
			if (!WordStack.empty() && WordStack.back() == "DOUSE" && CodeSymbol == "EXIT") {
				WordStack.pop_back();
			}
			if (symbol == "EXIT" || symbol == "DOVAR" || symbol == "EXECU") {
				WordStack.pop_back();
			} else {
				if (symbol == "DOLST") {
					WordStack.push_back(PossiblyColonWord);
				} else {
					PossiblyColonWord = symbol;
				}
			}
		}
	}
	if (WordStack.empty() && CodeSymbol == "") {
		logFormat(logLevel, "Words: empty");
	} else {
		std::string words;
		for (auto word: WordStack) {
			words += word + " ";
		}
		if (CodeSymbol != "") {
			words += "[" + CodeSymbol + "]";
		}
		logFormat(logLevel, "Words: %s", words.c_str());
	}

	// Turn off memory access diagnostics while we access memory...
	WORD32 oldflags = flags & DebugFlags_MemAccessDebugLevel;
	CLEAR_FLAGS(DebugFlags_MemAccessDebugLevel);

	// eForth registers from the workspace...
	WORD32 SPX = myMemory->getWord(Wdesc_WPtr(Wdesc) + 4); // Word[1]
	WORD32 IP = myMemory->getWord(Wdesc_WPtr(Wdesc) + 8);  // Word[2]
	WORD32 RP = myMemory->getWord(Wdesc_WPtr(Wdesc) + 12); // Word[3]
	logFormat(logLevel, "SP #%08X %c RP #%08X %c IP #%08X%s", SPX, (SPX == SPP ? 'E' : ' '), RP, (RP == RPP ? 'E' : ' '), IP, mySymbolTable->possibleSymbolString(IP).c_str());

	// Note: if the sizes of the stacks change, the i<XX will need changing in these loops:
	// Data stack...
	for (WORD32 i=0,a=SPP; SPX != 0 && a>SPX && i<176; i++,a-=4) {
       		WORD32 w = myMemory->getWord(a-4);
		logFormat(logLevel, "SP[%3d]@#%08X:#%08X%s", i, a-4, w, mySymbolTable->possibleSymbolString(w).c_str());
	}
	// Return stack...
	for (WORD32 i=0,a=RPP; RP != 0 && a>RP && i<64; i++,a-=4) {
		WORD32 w = myMemory->getWord(a-4);
		logFormat(logLevel, "RP[%3d]@#%08X:#%08X%s", i, a-4, w, mySymbolTable->possibleSymbolString(w).c_str());
	}

	// Restore memory access diagnostics.
	SET_FLAGS(oldflags);
}

// Disassemble from addr all full instructions up to addr+maxlen
// return number of bytes actually disassembled, i.e. don't
// disassemble a part instruction.
// Coalesce pfix/nfix
WORD32 CPU::disassembleRange(WORD32 addr, WORD32 maxlen) {
	char line[256];
	char misc[256];
	WORD32 caddr;
	WORD32 clen = 0;
	WORD32 retval = 0;
	WORD32 cOreg = 0;
	WORD32 oprStart = addr;
	WORD32 i;
	line[0] = '\0';
#if defined(PLATFORM_WINDOWS)
    sprintf_s(line, 256, "%08X ", addr);
#elif defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
    sprintf(line, "%08X ", addr);
#endif
	for (caddr = addr; caddr < addr + maxlen; caddr++) {
		BYTE8 b = myMemory->getInstruction(caddr);
		// Decode it
		BYTE8 cInstruction = b & 0xf0;
		cOreg |= (b & 0x0f);
#if defined(PLATFORM_WINDOWS)
        sprintf_s(misc, 256, "%02X ", b);
		strcat_s(line, 256, misc); // TODO: fix potential BUFFER OVERFLOW
#elif defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
        sprintf(misc, "%02X ", b);
		strcat(line, misc); // TODO: fix potential BUFFER OVERFLOW
#endif
		clen++;
		switch (cInstruction) {
			case D_pfix:
				cOreg <<= 4;
				if (clen > 8) {
					logWarn("More than 8 prefixes disassembled: this is either not code, or badly optimised code");
				}
				break;
			case D_nfix:
				cOreg = (~cOreg) << 4;
				if (clen > 8) {
					logWarn("More than 8 prefixes disassembled: this is either not code, or badly optimised code");
				}
				break;
			case D_opr:
				// Unless we interpret the code before this, we
				// have no way of knowing what Areg might be
				// set to. This is only a problem when
				// cInstruction is fpentry, and Areg codes for
				// a floating point instruction. So just say 0,
				// for Areg, and this'll cause ?fp? to be used.
#if defined(PLATFORM_WINDOWS)
				for (i = 0; i < 8 - clen; i++) {
					strcat_s(line, 256, "   ");
				}
				strcat_s(line, 256, disassembleIndirectOperation(cOreg, 0)); // TODO: fix potential BUFFER OVERFLOW
#elif defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
				for (i = 0; i < 8 - clen; i++) {
					strcat(line, "   ");
				}
				strcat(line, disassembleIndirectOperation(cOreg, 0)); // TODO: fix potential BUFFER OVERFLOW
#endif
				logInfo(line);
				// initialise for next op
				oprStart = caddr + 1;
#if defined(PLATFORM_WINDOWS)
                sprintf_s(line, 256, "%08X ", oprStart);
#elif defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
                sprintf(line, "%08X ", oprStart);
#endif
                cOreg = 0;
				retval += clen;
				clen = 0;
				break;
			default: // another direct instruction
#if defined(PLATFORM_WINDOWS)
				for (i = 0; i < 8 - clen; i++) {
					strcat_s(line, 256, "   ");
				}
				strcat_s(line, 256, disassembleDirectOperation(cInstruction, cOreg)); // TODO: fix potential BUFFER OVERFLOW
#elif defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
				for (i = 0; i < 8 - clen; i++) {
					strcat(line, "   ");
				}
				strcat(line, disassembleDirectOperation(cInstruction, cOreg)); // TODO: fix potential BUFFER OVERFLOW
#endif
				logInfo(line);
				// initialise for next op
				oprStart = caddr + 1;
#if defined(PLATFORM_WINDOWS)
                sprintf_s(line, 256, "%08X ", oprStart);
#elif defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
                sprintf(line, "%08X ", oprStart);
#endif
                cOreg = 0;
				retval += clen;
				clen = 0;
				break;
		}
	}
	return retval;
}

void CPU::disassembleCurrInstruction(int logLevel) {
	switch (Instruction) {
		case D_pfix:
		case D_nfix:
			if ((flags & DebugFlags_DebugLevel) >= Debug_OprCodes) {
				// The IPtr of this instruction is needed for prefixes
				logFormat(logLevel, "#%08X%s: %s", IPtr - 1, 
					mySymbolTable->possibleSymbol(IPtr - 1),
					disassembleDirectOperation(Instruction, Oreg));
			}
			break;
		case D_opr:
			if ((flags & DebugFlags_DebugLevel) >= Debug_Disasm) {
				if ((flags & DebugFlags_DebugLevel) >= Debug_OprCodes) {
					logFormat(logLevel, ">%08X%s: %s", IPtr - 1,
							mySymbolTable->possibleSymbol(IPtr - 1),
							disassembleIndirectOperation(Oreg, Areg));
				} else {
					logFormat(logLevel, "#%08X: %s",
							InstructionStartIPtr,
							disassembleIndirectOperation(Oreg, Areg));
				}
			}
			break;
		default: // another direct instruction
			if ((flags & DebugFlags_DebugLevel) >= Debug_Disasm) {
				if ((flags & DebugFlags_DebugLevel) >= Debug_OprCodes) {
					logFormat(logLevel, ">%08X%s: %s", IPtr - 1,
							mySymbolTable->possibleSymbol(IPtr - 1),
							disassembleDirectOperation(Instruction, Oreg));
				} else {
					logFormat(logLevel, "#%08X: %s",
							InstructionStartIPtr,
							disassembleDirectOperation(Instruction, Oreg));
				}
			}
			break;
	}

}

void dumpFlags() {
	logInfoF("F %08X", flags);
	if (IS_FLAG_SET(EmulatorState_ErrorFlag))
		logInfo("-- ERROR");
	if (IS_FLAG_SET(EmulatorState_HaltOnError))
		logInfo("-- HALT ON ERROR");
	if (IS_FLAG_SET(EmulatorState_FErrorFlag))
		logInfo("-- FLOATING POINT ERROR");
	if (IS_FLAG_SET(EmulatorState_J0Break))
		logInfo("-- JUMP 0 BREAK");
	if (IS_FLAG_SET(EmulatorState_DeschedulePending))
		logInfo("-- DESCHEDULE PENDING");
	if (IS_FLAG_SET(EmulatorState_DescheduleRequired))
		logInfo("-- DESCHEDULE REQUIRED");
	if (IS_FLAG_SET(EmulatorState_Interrupt))
		logInfo("-- INTERRUPT");
	if (IS_FLAG_SET(EmulatorState_BadInstruction))
		logInfo("-- BAD INSTRUCTION");
	if (IS_FLAG_SET(EmulatorState_QueueInstruction))
		logInfo("-- QUEUE INSTRUCTION");
	if (IS_FLAG_SET(EmulatorState_TimerInstruction))
		logInfo("-- TIMER INSTRUCTION");
}

void CPU::showBreakpointAddresses() {
    if (BreakpointAddresses.empty()) {
        logInfo("No breakpoints are set");
    } else {
        for (set<WORD32>::const_iterator iter = BreakpointAddresses.begin();
             iter != BreakpointAddresses.end(); iter++) {
            logInfoF("Breakpoint %08X", *iter);
        }
    }
}

// Monitor. Return true to single step to next instruction; false to quit emulator or monitor (check flags).
inline bool CPU::monitor(void) {
	char instr[80];
	size_t len;
	WORD32 a1, a2;
	// The instruction has just been disassembled, but we allow for
	// other commands to be issued. Return exits the monitor,
	// causing interpretation to continue.
	while (true) {
		logPrompt();
		getInput(instr, 80);
		len = strlen(instr);
		if (len > 0) {
			instr[len - 1] = '\0';
		}
		len --;
		if (len == 0) {
			return true;
		}
		if (strcmp(instr, "h") == 0 || strcmp(instr, "?") == 0) {
			//       12345678901234567890123456789012345678901234567890123456789012345678901234567890
			logInfo("Monitor commands:");
			logInfo("ci                   disassemble current instruction");
			logInfo("di [addr [len]]      disassemble from addr (hex) for len (hex) bytes");
			logInfo("db [addr [len]]      dump hex bytes/ASCII from addr (hex) for len (hex) bytes");
			logInfo("dw [addr [len]]      dump hex words/ASCII from addr (hex) for len (hex) words");
			logInfo("w [len]              dump hex words/ASCII from Wptr for len (hex) words;");
			logInfo("                     default len = current Workspace size");
			logInfo("b addr  or  b+ addr  add addr (hex) as a breakpoint");
			logInfo("b- addr              remove addr (hex) as a breakpoint");
			logInfo("b?  or  b <no args>  display all breakpoint addresses");
			logInfo("<return>             single-step current instruction");
			logInfo("r                    display all registers (depends on register display flags)");
			logInfo("rq                   display queue registers");
			logInfo("rc                   display clock registers");
			logInfo("f                    display flags");
			logInfo("s                    display all state: registers, flags, current disassembly");
			logInfo("q                    quit emulator");
            logInfo("t                    toggle disassembly of opr/memory R/W");
			logInfo("g                    'go': quit monitor, continue interpretation");
			logInfo("                     (until any breakpoints reached)");
		}
		if (strcmp(instr, "r") == 0) {
			DumpRegs(LOGLEVEL_INFO);
			if (IS_FLAG_SET(EmulatorState_QueueInstruction))
				DumpQueueRegs(LOGLEVEL_INFO);
			if (IS_FLAG_SET(EmulatorState_TimerInstruction))
				DumpClockRegs(LOGLEVEL_INFO, InstCycles+MemCycles);
		} else if (strcmp(instr, "rq") == 0) {
			DumpQueueRegs(LOGLEVEL_INFO);
		} else if (strcmp(instr, "rc") == 0) {
			DumpClockRegs(LOGLEVEL_INFO, InstCycles+MemCycles);
		} else if (strcmp(instr, "ci") == 0) {
			disassembleCurrInstruction(LOGLEVEL_INFO);
		} else if (strncmp("di", instr, 2) == 0) {
#if defined(PLATFORM_WINDOWS)
			if (sscanf_s(instr, "di %x %x", &a1, &a2) == 2) {
#elif defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
			if (sscanf(instr, "di %x %x", &a1, &a2) == 2) {
#endif
				CurrDisasmAddress = a1;
				CurrDisasmLen = a2;
#if defined(PLATFORM_WINDOWS)
			} else if (sscanf_s(instr, "di %x", &a1) == 1) {
#elif defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
            } else if (sscanf(instr, "di %x", &a1) == 1) {
#endif
				CurrDisasmAddress = a1;
			}
			CurrDisasmAddress += disassembleRange(CurrDisasmAddress, CurrDisasmLen);
		} else if (strncmp("db", instr, 2) == 0) {
#if defined(PLATFORM_WINDOWS)
			if (sscanf_s(instr, "db %x %x", &a1, &a2) == 2) {
#elif defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
			if (sscanf(instr, "db %x %x", &a1, &a2) == 2) {
#endif
				CurrDataAddress = a1;
				CurrDataLen = a2;
#if defined(PLATFORM_WINDOWS)
            } else if (sscanf_s(instr, "db %x", &a1) == 1) {
#elif defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
            } else if (sscanf(instr, "db %x", &a1) == 1) {
#endif
				CurrDataAddress = a1;
			}
			//logInfoF("db addr %08X len %08X", CurrDataAddress, CurrDataLen);
			myMemory->hexDump(CurrDataAddress, CurrDataLen);
			CurrDataAddress += CurrDataLen;
		} else if (strncmp("dw", instr, 2) == 0) {
#if defined(PLATFORM_WINDOWS)
			if (sscanf_s(instr, "dw %x %x", &a1, &a2) == 2) {
#elif defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
			if (sscanf(instr, "dw %x %x", &a1, &a2) == 2) {
#endif
				CurrDataAddress = a1;
				CurrDataLen = a2;
#if defined(PLATFORM_WINDOWS)
            } else if (sscanf_s(instr, "dw %x", &a1) == 1) {
#elif defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
			} else if (sscanf(instr, "dw %x", &a1) == 1) {
#endif
				CurrDataAddress = a1;
			}
			//logInfoF("dw addr %08X len %08X", CurrDataAddress, CurrDataLen);
			myMemory->hexDumpWords(CurrDataAddress, CurrDataLen);
			CurrDataAddress += CurrDataLen;
        } else if (strncmp("w", instr, 1) == 0) {
            CurrDataAddress = Wdesc_WPtr(Wdesc);
#if defined(PLATFORM_WINDOWS)
			if (sscanf_s(instr, "w %x", &a1) == 1) {
#elif defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
            if (sscanf(instr, "w %x", &a1) == 1) {
#endif
                CurrDataLen = a1 << 2;
            } else {
                CurrDataLen = LastAjwInBytes;
            }
            myMemory->hexDumpWords(CurrDataAddress, CurrDataLen);
        } else if (strncmp("b?", instr, 2) == 0) {
		    showBreakpointAddresses();
		} else if (strncmp("b+", instr, 2) == 0) {
#if defined(PLATFORM_WINDOWS)
			if (sscanf_s(instr, "b+ %x", &a1) == 1) {
#elif defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
			if (sscanf(instr, "b+ %x", &a1) == 1) {
#endif
				addBreakpoint(a1);
			}
		} else if (strncmp("b-", instr, 2) == 0) {
#if defined(PLATFORM_WINDOWS)
			if (sscanf_s(instr, "b- %x", &a1) == 1) {
#elif defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
			if (sscanf(instr, "b- %x", &a1) == 1) {
#endif
				removeBreakpoint(a1);
			}
        } else if (strncmp("b", instr, 1) == 0) {
#if defined(PLATFORM_WINDOWS)
			if (sscanf_s(instr, "b %x", &a1) == 1) {
#elif defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
			if (sscanf(instr, "b %x", &a1) == 1) {
#endif
				addBreakpoint(a1);
			} else {
				showBreakpointAddresses();
			}
		} else if (strcmp(instr, "s") == 0) {
			DumpRegs(LOGLEVEL_DEBUG);
			DumpQueueRegs(LOGLEVEL_DEBUG);
			DumpClockRegs(LOGLEVEL_DEBUG, InstCycles+MemCycles);
			dumpFlags();
			disassembleCurrInstruction(LOGLEVEL_INFO);
		} else if (strcmp(instr, "f") == 0) {
			dumpFlags();
		} else if (strcmp(instr, "q") == 0) {
            SET_FLAGS(EmulatorState_Terminate);
            return false;
        } else if (strcmp(instr, "t") == 0) {
            if (IS_FLAG_SET(Debug_OprCodes)) {
                logInfo("Stopping disassembly");
                CLEAR_FLAGS(Debug_OprCodes);
                CLEAR_FLAGS(MemAccessDebug_ReadWriteData);
            } else {
                logInfo("Starting disassembly");
                SET_FLAGS(Debug_OprCodes);
                SET_FLAGS(MemAccessDebug_ReadWriteData);
            }
        } else if (strcmp(instr, "g") == 0) {
			CLEAR_FLAGS(DebugFlags_Monitor);
			return false;
		} else {
			logWarnF("Unknown monitor command '%s'", instr);
		}
    logInfo("");
	}
}

bool CPU::swapContextForBreakpointInstruction(void) {
	bool hiPriority = Wdesc_HiPriority(Wdesc);
	WORD32 BreakpointContext = hiPriority ? MemStart : MemStart + 2;
	WORD32 currWdesc = Wdesc;
	WORD32 currIPtr = IPtr;
	Wdesc = myMemory->getWord(BreakpointContext);
	IPtr = myMemory->getWord(BreakpointContext + 4);
	myMemory->setWord(BreakpointContext, currWdesc);
	myMemory->setWord(BreakpointContext + 4, currIPtr);
	SET_FLAGS(EmulatorState_BreakpointInstruction);
	return hiPriority;
}

inline void CPU::interpret(void) {
	bool hitBreakpoint = IS_FLAG_SET(EmulatorState_BreakpointInstruction) ||
						 BreakpointAddresses.count(IPtr) == 1;
	// Fetch the current instruction
	CurrInstruction = myMemory->getInstruction(IPtr++);
	// Decode it
	Instruction = CurrInstruction & 0xf0;
	Oreg |= (CurrInstruction & 0x0f);
	//logDebugF("CurrInstruction =0x%02X Oreg=0x%08X", CurrInstruction, Oreg);
	// Disassemble it
	if (IS_FLAG_SET(DebugFlags_DebugLevel | DebugFlags_Monitor)) {
		disassembleCurrInstruction(LOGLEVEL_DEBUG);
	}
	if (hitBreakpoint) {
		logInfo("*** BREAKPOINT");
		SET_FLAGS(DebugFlags_Monitor); // Enable monitor mode, until you exit it with 'g' (or 'q').
		CLEAR_FLAGS(EmulatorState_BreakpointInstruction); // Will be set on the next breakpoint instruction.
	}
	if (hitBreakpoint || IS_FLAG_SET(DebugFlags_Monitor)) {
		if (!monitor()) {
			return; // it's terminated if it returns false
		}
	}
	// Execute instruction, assuming one cycle per instruction unless
	// set otherwise.
	InstCycles = 1;
	// Clear pre-execute flags
	flags &= FlagMask;
	flags |= InterpFlagSet;
	// No schedule required as of yet. This will point to a process's
	// workspace if that process should be scheduled, after the
	// instruction has executed. 0 is a valid workspace, so initialise this to NotProcess_p (mint).
	ScheduleWdesc = NotProcess_p;
	// Interpret... save Oreg in case we have a bad instruction
	OldOreg = Oreg;
	switch (Instruction) {
		case D_j: // jump
			if (Oreg == 0) {
				if (IS_FLAG_SET(EmulatorState_TVS)) {
					logInfo("j 0 in TVS; terminating");
					SET_FLAGS(EmulatorState_Terminate);
				}
				if (IS_FLAG_SET(EmulatorState_J0Break)) {
					logInfo("*** Breakpoint (j 0) ***");
					bool hiPriority = swapContextForBreakpointInstruction();
					InstCycles = hiPriority ? 11 : 13;
				} else {
					logWarn("j: 0, but j 0 break not set");
				}
			}
			if (Oreg == -2) {
				logWarn("j: infinite loop - premature end?");
			} else {
				IPtr += Oreg;
				InstCycles = 3;
				if IS_FLAG_SET(EmulatorState_DeschedulePending) 
					SET_FLAGS(EmulatorState_DescheduleRequired);
				else
					CLEAR_FLAGS(EmulatorState_DescheduleRequired);
			}
			break;

		case D_ldlp: // load local pointer
			PUSH(Wdesc_WPtr(Wdesc) + (Oreg << 2));
			break;

		case D_pfix: // prefix
			Oreg <<= 4;
			break;

		case D_ldnl: // load non local
			Areg = myMemory->getWord(Areg + (Oreg << 2));
			InstCycles++;
			break;

		case D_ldc: // load constant
			PUSH(Oreg);
			break;

		case D_ldnlp: // load non local pointer
			Areg += Oreg << 2;
			break;

		case D_nfix: // negative prefix
			Oreg = (~Oreg) << 4;
			break;

		case D_ldl: // load local
			PUSH(myMemory->getWord(Wdesc_WPtr(Wdesc) + (Oreg << 2)));
			InstCycles++;
			break;

		case D_adc: { // add constant checked
				WORD32 AregSign = Areg & SignBit;
				WORD32 OregSign = Oreg & SignBit;
				WORD32 result = Areg + Oreg;
				Areg = result;
				WORD32 resultSign = Areg & SignBit;
				if (AregSign == OregSign && AregSign != resultSign) {
					SET_FLAGS(EmulatorState_ErrorFlag);
				}
			}
			break;

		case D_call: // call
			InstCycles = 7;
			Wdesc -= 16;
			myMemory->setWord(Wdesc_WPtr(Wdesc), IPtr);
			myMemory->setWord(Wdesc_WPtr(Wdesc) + 4, Areg);
			myMemory->setWord(Wdesc_WPtr(Wdesc) + 8, Breg);
			myMemory->setWord(Wdesc_WPtr(Wdesc) + 12, Creg);
			Areg = IPtr; // cwg says NextInst
			Breg = Creg; // Spec says 'undefined' but this is what happens. Creg becomes 'undefined'.
			IPtr += Oreg;
			break;

		case D_cj: // conditional jump
			if (Areg == 0) {
				IPtr += Oreg;
				InstCycles = 4;
			} else {
				InstCycles++;
				DROP();
			}
			break;

		case D_ajw: // adjust workspace
            LastAjwInBytes = Oreg << 2; // convenience store of current workspace size (in bytes) for the monitor w command
			Wdesc += LastAjwInBytes;
			break;

		case D_eqc: // equals constant
			Areg = (Areg == Oreg);
			InstCycles++;
			break;

		case D_stl: // store local
			myMemory->setWord(Wdesc_WPtr(Wdesc) + (Oreg << 2), Areg);
			DROP();
			break;

		case D_stnl: // store non local
			myMemory->setWord(Areg + (Oreg << 2), Breg);
			Areg = Creg;
			InstCycles++;
			break;

		case D_opr: // operate
			switch (Oreg) {

				case O_rev: { // reverse
						WORD32 t = Areg;
						Areg = Breg;
						Breg = t;
					}
					break;

				case O_add: { // add checked
						WORD32 BregSign = Breg & SignBit;
						WORD32 AregSign = Areg & SignBit;
						Areg += Breg;
						WORD32 resultSign = Areg & SignBit;
						if (BregSign == AregSign && BregSign != resultSign) {
							SET_FLAGS(EmulatorState_ErrorFlag);
						}
						Breg = Creg;
					}
					break;

				case O_sub: { // subtract checked
						WORD32 BregSign = Breg & SignBit;
						WORD32 AregSign = Areg & SignBit;
						Areg = Breg - Areg;
						WORD32 resultSign = Areg & SignBit;
						if (BregSign != AregSign && BregSign == resultSign) {
							SET_FLAGS(EmulatorState_ErrorFlag);
						}
						Breg = Creg;
					}
					break;
                
				case O_mul: { // multiply checked
						// From "Hacker's Delight, 2nd ed", Henry S. Warren. p32
						// "The test can be simplified if unsigned division is
						// available..."

						// logInfoF("mul: Areg %08X Breg %08X", Areg, Breg);

						WORD32 c = (Areg & SignBit) == (Breg & SignBit) ?
							0x7FFFFFFF : // 2^31 - 1
							0x80000000 ; // 2^31
						WORD32 uAreg = (Areg & SignBit) ? ~Areg + 1 : Areg;
						WORD32 uBreg = (Breg & SignBit) ? ~Breg + 1 : Breg;
						bool overflow = Breg != 0 && (uAreg > (c / uBreg));
						// logInfoF("mul: c: %08X uAreg %08X uBreg %08X", c, uAreg, uBreg);
						if (overflow) {
							SET_FLAGS(EmulatorState_ErrorFlag);
							// std::cout << "mul: overflow" << std::endl;
						}
						Areg *= Breg;

						// logInfoF("mul: result: %08X\n", Areg);
						Breg = Creg;
						InstCycles = BitsPerWord + 6;
					}
					break;

				case O_div: // divide 
					if ((Areg == 0) || ((Areg == 0xFFFFFFFF) && (Breg == SignBit))) {
						SET_FLAGS(EmulatorState_ErrorFlag);
						Breg = Creg;
					} else {
						WORD32 absAreg = abs((SWORD32)Areg);
						WORD32 absBreg = abs((SWORD32)Breg);
						Areg = (SWORD32)Breg / (SWORD32)Areg;
						Breg = Creg;
						// With thanks to Mike Bruestle's analysis & TVS
						Creg = absBreg - (abs((SWORD32)Areg) | 1) * absAreg;
						InstCycles = BitsPerWord + 10;
					}
					break;

				case O_rem: // remainder
					if (Areg == 0) {
						Areg = Breg;
						Breg = Creg;
						SET_FLAGS(EmulatorState_ErrorFlag);
					} else {
						if ((Areg == 0xFFFFFFFF) && (Breg == SignBit)) {
							Areg = 0;
							Breg = Creg;
							Creg = 0;
						} else {
							Areg = (SWORD32)Breg % (SWORD32)Areg;
							Breg = Creg;
							Creg = abs((SWORD32)Areg);
						}
					}
					InstCycles = BitsPerWord + 5;
					break;

				case O_sum: // sum unchecked
					Areg += Breg;
					Breg = Creg;
					break;
             
				case O_diff: // difference unchecked
					Areg = Breg - Areg;
					Breg = Creg;
					break;

				case O_prod: // product unchecked
					InstCycles = HighestSetBit(Areg) + 4;
					Areg *= Breg;
					Breg = Creg;
					break;

				case O_and: // bitwise and
					Areg &= Breg;
					Breg = Creg;
					break;

				case O_or: // bitwise or
					Areg |= Breg;
					Breg = Creg;
					break;

				case O_xor: // bitwise exclusive or
					Areg ^= Breg; 
					Breg = Creg;
					break;

				case O_not: // bitwise complement
					Areg = ~Areg;
					break;

				case O_shl: // shift left unsigned
					InstCycles = Areg + 2;
					if (Areg >= BitsPerWord) {
						logDebug("shl: Areg >= 32");
						InstCycles++;
						Areg = 0;
					} else {
						Areg = Breg << Areg;
					}
					Breg = Creg;
					break;

				case O_shr: // shift right unsigned
					InstCycles = Areg + 2;
					if (Areg >= BitsPerWord) {
						logDebug("shr: Areg >= 32");
						InstCycles++;
						Areg = Breg = 0;
					} else {
						Areg = Breg >> Areg;
					}
					Breg = Creg;
					break;

				case O_gt: { // greater than signed
						SWORD32 sAreg = (SWORD32)Areg;
						SWORD32 sBreg = (SWORD32)Breg;
						Areg = (sBreg > sAreg);
						Breg = Creg;
						InstCycles++;
					}
					break;

				case O_lend: { // loop end
						WORD32 Count = myMemory->getWord(Breg + 4);
						myMemory->setWord(Breg + 4, Count - 1);
						if (Count > 1) { // loop back
							myMemory->setWord(Breg, myMemory->getWord(Breg) + 1);
							IPtr -= Areg;
							InstCycles = 10;
						} else {
							InstCycles = 5;
						}
						// TODO Should the deschedule only occur when looping back?
						// See CWG p40, "[descheduling] will occur after the next j or lend instruction executed."
						// What does 'executed' mean? Every time, or only when loop effected?
						// Instruction set summary seems to imply this...
						// If there's a deschedule pending, set the required flag, so
						// that it actually occurs.
						if (IS_FLAG_SET(EmulatorState_DeschedulePending)) {
							SET_FLAGS(EmulatorState_DescheduleRequired);
						}
					}
					break;

				case O_bcnt: // byte count
					Areg <<= 2;
					InstCycles++;
					break;

				case O_wcnt: // word count
					Creg = Breg;
					Breg = Areg & ByteSelectMask;
					Areg  = ((SWORD32)Areg) >> 2;
					InstCycles = 5;
					break;

				case O_ldpi: // Load pointer to instruction
					Areg += IPtr;
					InstCycles++;
					break;

				case O_mint: // Minimum integer
					PUSH(NotProcess_p);
					break;

				case O_bsub: // byte subscript Areg[Breg]
					Areg += Breg;
					Breg = Creg;
					break;

				case O_wsub: // word subscript
					Areg += (Breg << 2);
					Breg = Creg;
					InstCycles++;
					break;

				case O_move: // move message
					if ( (! ((Creg <= Breg) && (Breg < (Creg + Areg))) ) &&
						(! ((Breg <= Creg) && (Creg < (Breg + Areg))) ) ) {
						InstCycles = 8;
						SET_FLAGS(EmulatorState_Interrupt);
						// Copies Areg bytes starting
						// at Creg to the block
						// starting at Breg
						myMemory->blockCopy(Areg, Creg, Breg);
					} else {
						logWarn("move: blocks overlap");
					}
					break;

				case O_in: { // input message
						// Input message of length Areg bytes from channel pointed
						// to by Breg to memory at Creg. Takes 2w+18 iff
						// communication proceeds, or 20 iff communication waits, and
						// DescheduleFlag is set.
						// See memory::blockCopy for details of timing.
						InstCycles = 18;
						// TODO: should we deschedule only when communication waits?
						// Instruction set summary seems to imply this..
						SET_FLAGS(EmulatorState_Interrupt);
						Link *myLink = NULL;
						switch (Breg) {
							// Need to be able to do a blockcopy-type operation from the
							// Node Server for proper timing..... or fudge the memory
							// cycle count mechanism for this special case.
							case Link0Input:
								myLink = myLinks[0];
								break;
							case Link1Input:
								myLink = myLinks[1];
								break;
							case Link2Input:
								myLink = myLinks[2];
								break;
							case Link3Input:
								myLink = myLinks[3];
								break;
							default: // Do input from memory channel
								if (myMemory->isLegalMemory(Creg) &&
								    myMemory->isLegalMemory(Creg + Areg)) {
									WORD32 WorkSpace = myMemory->getWord(Breg);
									if (Wdesc_WPtr(WorkSpace) == NotProcess_p) {
										// This in reached the rendezvous first
										myMemory->setWord(W_POINTER(Wdesc), Creg);
										myMemory->setWord(Breg, Wdesc);
										InstCycles = 20;
										SET_FLAGS(EmulatorState_DescheduleRequired);
									} else {
										// The out reached the rendezvous first
										WORD32 ChanAddr = myMemory->getWord(W_POINTER(WorkSpace));
										// Copy Areg bytes from ChanAddr to Creg
										myMemory->blockCopy(Areg, ChanAddr, Creg);
										// Reset channel to unused
										myMemory->setWord(Breg, NotProcess_p);
										// Request a schedule of the process at WorkSpace
										ScheduleWdesc = WorkSpace;
									}
								} else {
									logWarnF("in to bad memory area Creg=%08X Areg=%08X", Creg, Areg);
								}
								break;
						}
						// Now handle input from real links
						if (myLink != NULL) {
							try {
								WORD32 i;
								for (i = 0; i < Areg; i++)  {
									myMemory->setByte(Creg + i, myLink->readByte());
								}
							} catch (exception &e) {
								logErrorF("in failed to read byte from link %d: %s", myLink->getLinkNo(), e.what());
								SET_FLAGS(EmulatorState_Terminate);
							}
						}
					}
					break;

				case O_out: { // output message
						// Output message of length Areg bytes to the channel
						// pointed to by Breg from memory at Creg
						// takes 2w+20 iff communication proceeds, or 20 iff
						// communication waits, and DescheduleFlag is set.
						// See memory::blockCopy for details of timing.
						InstCycles = 20 ;
						// TODO: should we deschedule only when communication waits?
						// Instruction set summary seems to imply this..
						SET_FLAGS(EmulatorState_Interrupt);
						Link *myLink = NULL;
						switch (Breg) {
							case Link0Output:
								myLink = myLinks[0];
								break;
							case Link1Output:
								myLink = myLinks[1];
								break;
							case Link2Output:
								myLink = myLinks[2];
								break;
							case Link3Output:
								myLink = myLinks[3];
								break;
							default: // Do output to memory channel
								if (myMemory->isLegalMemory(Creg) &&
								    myMemory->isLegalMemory(Creg + Areg)) {
									WORD32 WorkSpace = myMemory->getWord(Breg);
									if (Wdesc_WPtr(WorkSpace) == NotProcess_p) {
										// This out reached the rendezvous first
										myMemory->setWord(W_POINTER(Wdesc), Creg);
										myMemory->setWord(Breg, Wdesc);
										InstCycles = 20;
										SET_FLAGS(EmulatorState_DescheduleRequired);
									} else {
										WORD32 ChanAddr = myMemory->getWord(W_POINTER(WorkSpace));
										// The in reached the rendezvous first
										// Copy Areg bytes from Creg to ChanAddr
										myMemory->blockCopy(Areg, Creg, ChanAddr);
										// Reset channel to unused
										myMemory->setWord(Breg, NotProcess_p);
										// Request a schedule of the process at WorkSpace
										ScheduleWdesc = WorkSpace;
									}
								} else {
									logWarnF("out from bad memory area Creg=%08X Areg=%08X", Creg, Areg);
								}
								break;
						}
						// Now handle output to real links
						if (myLink != NULL) {
							try {
								WORD32 i;
								// Again, need to do something about blockcopy over
								// the IServer link.... see O_in processing.
								for (i = 0; i < Areg; i++) {
									myLink->writeByte(myMemory->getByte(Creg + i));
								}
							} catch (exception &e) {
								logErrorF("out failed to write byte to link %d: %s", myLink->getLinkNo(), e.what());
								SET_FLAGS(EmulatorState_Terminate);
							}
						}
					}
					break;

				case O_lb: // load byte
					Areg = myMemory->getByte(Areg);
					InstCycles = 5;
					break;

				case O_sb: // store byte
					myMemory->setByte(Areg, (BYTE8)Breg & 0xff);
   					InstCycles = 4;
					break;

				case O_outbyte: { // output byte
						InstCycles = 25;
						Link *myLink = NULL;
						switch (Breg) {
							case Link0Output:
								myLink = myLinks[0];
								break;
							case Link1Output :
								myLink = myLinks[1];
								break;
							case Link2Output :
								myLink = myLinks[2];
								break;
							case Link3Output :
								myLink = myLinks[3];
								break;
							default: { // Do output to memory channel
								WORD32 WorkSpace;
								WorkSpace = myMemory->getWord(Breg);
								if (Wdesc_WPtr(WorkSpace) == NotProcess_p) {
									// The outbyte got to the rendezvous
									// first.. Store Areg in the workspace
									// temporary variable...
									myMemory->setByte(W_TEMP(Wdesc), (BYTE8)Areg & 0xff);
									myMemory->setWord(W_POINTER(Wdesc), Wdesc_WPtr(Wdesc));
									myMemory->setWord(Breg, Wdesc);
									SET_FLAGS(EmulatorState_DescheduleRequired);
								} else {
									// The in got to the rendezvos first
									myMemory->setByte(myMemory->getWord(W_POINTER(WorkSpace)), (BYTE8)Areg & 0xff);
									myMemory->setWord(Breg, NotProcess_p);
									// Schedule the process at WorkSpace
									ScheduleWdesc = WorkSpace;
								}
								}
								break;
						}
						// Now handle output to real links
						if (myLink != NULL) {
							try {
								myLink->writeByte((BYTE8)Areg & 0xff);
							} catch (exception &e) {
								logErrorF("outbyte failed to write byte to link %d: %s", myLink->getLinkNo(), e.what());
								SET_FLAGS(EmulatorState_Terminate);
							}
						}
					}
					break;

				case O_outword: { // output word
						Link *myLink = NULL;
						InstCycles = 25;
						switch (Breg) {
							case Link0Output:
								myLink = myLinks[0];
								break;
							case Link1Output :
								myLink = myLinks[1];
								break;
							case Link2Output :
								myLink = myLinks[2];
								break;
							case Link3Output :
								myLink = myLinks[3];
								break;
							default: { // Do output to memory channel */
								WORD32 WorkSpace;
								WorkSpace = myMemory->getWord(Breg);
								if (Wdesc_WPtr(WorkSpace) == NotProcess_p) {
									// The outword got to the rendezvous
									// first.. Store Areg in the workspace
									// temporary variable...
									myMemory->setWord(W_TEMP(Wdesc),Areg);
									myMemory->setWord(W_POINTER(Wdesc), Wdesc_WPtr(Wdesc));
									myMemory->setWord(Breg, Wdesc);
									SET_FLAGS(EmulatorState_DescheduleRequired);
								} else {
									// The inword got to the rendezvous first
									myMemory->setWord(myMemory->getWord(W_POINTER(WorkSpace)), Areg);
									myMemory->setWord(Breg, NotProcess_p);
									// Schedule the process at WorkSpace
									ScheduleWdesc = WorkSpace;
								}
								}
								break;
						}
						// Now handle output to real links
						if (myLink != NULL) {
							try {
								myLink->writeWord(Areg);
							} catch (exception &e) {
								logErrorF("outword failed to write word to link %d: %s", myLink->getLinkNo(), e.what());
								SET_FLAGS(EmulatorState_Terminate);
							}
						}
					}
					break;

				case O_gcall: { // general call
						WORD32 t = Areg;
						Areg = IPtr;
						IPtr = t;
						InstCycles = 4;
					}
					break;

				case O_gajw: { // general adjust workspace
						WORD32 t = Areg;
						if ((Areg & ByteSelectMask) != (Wdesc & ByteSelectMask)) {
							logWarn("gajw: Attempting to change priority");
						}
						Areg = Wdesc;
						Wdesc = (t & WordMask) | (Wdesc & ByteSelectMask);
						InstCycles++;
					}
					break;

				case O_ret: // return
					IPtr = myMemory->getWord(Wdesc_WPtr(Wdesc));
					Wdesc += 16;
					InstCycles = 5;
					break;

				case O_startp: // start process
					// Add process with workspace Areg and instruction pointer at
					// offset of Breg bytes from IPtr to current priority process queue
					myMemory->setWord(W_IPTR(Areg), IPtr + Breg);
					// Request a schedule of the process at Areg
   					ScheduleWdesc = Wdesc_WPtr(Areg) | Wdesc_Priority(Wdesc);
					InstCycles = 12;
					break;

				case O_endp: { // end process
						WORD32 Count;
						InstCycles = 13;
						Count = myMemory->getWord(Areg + 4);
						myMemory->setWord(Areg + 4, Count - 1);
						if (Count == 1) {
							// Continue as process with waiting workspace Areg'
							if ((Wdesc & ByteSelectMask) != (Areg & ByteSelectMask)) {
								logWarn("endp: Attempting to change priority");
							}
							Wdesc = Wdesc_WPtr(Areg) | Wdesc_Priority(Wdesc);
							IPtr = myMemory->getWord(Wdesc_WPtr(Wdesc));
						} else {
							// Start next waiting process
							SET_FLAGS(EmulatorState_DescheduleRequired);
						}
					}
					break;

				case O_runp: // run process. 
					// Add the process with descriptor Areg to appropriate process queue 
					ScheduleWdesc = Areg;
					InstCycles = 10;
					break;

				case O_stopp: // stop process
					myMemory->setWord(W_IPTR(Wdesc), IPtr);
					SET_FLAGS(EmulatorState_DescheduleRequired);
					InstCycles = 11;
					break;

				case O_ldpri: // load priority
					PUSH(Wdesc_Priority(Wdesc));
					break;

				case O_ldtimer: // load timer
					InstCycles++;
					SET_FLAGS(EmulatorState_TimerInstruction);
					PUSH(Wdesc_HiPriority(Wdesc) ? HiClock : LoClock);
					break;

				case O_csub0: // check subscript from 0
					if (Breg >= Areg) {
						SET_FLAGS(EmulatorState_ErrorFlag);
					}
					InstCycles++;
					DROP();
					break;

				case O_ccnt1: // check count from 1
					if ((Breg == 0) || (Breg > Areg)) {
						SET_FLAGS(EmulatorState_ErrorFlag);
					}
					InstCycles = 3;
					DROP();
					break;

				case O_testerr: // test error flag false and clear
					InstCycles = 3; // this assumes worst case
					PUSH(IS_FLAG_CLEAR(EmulatorState_ErrorFlag));
					CLEAR_FLAGS(EmulatorState_ErrorFlag);
					break;

				case O_stoperr: // stop on error
					if (IS_FLAG_SET(EmulatorState_ErrorFlag)) {
						logWarn("stoperr: ErrorFlag is set. Deschedule?");
						SET_FLAGS(EmulatorState_DescheduleRequired);
						InstCycles++; // depends on error state (CWG errata)
					}
					break;

				case O_seterr: // set error flag
					SET_FLAGS(EmulatorState_ErrorFlag);
					break;

				case O_xword: // extend to word (cwg p142 & sec 5.8.1)
					logWarn("xword: TVS fail");
				    // Simplified version taken from Transputer Assembly Language programming
					if ( Breg < Areg ) {
						Areg = Breg;
					} else {
						Areg = Breg - ( Areg << 1);
					}
					InstCycles = 4;
					Breg = Creg;
					break;
				case O_cword: { // check word
						// Areg must be a power of 2; ie has a single bit
						int bits = 0, t = Areg;
						for (int i=0; i<32; i++) {
							if ((t & 1) == 1) {
								bits++;
							}
							t >>= 1;
						}
						if (bits == 1 && Areg != SignBit && 
							(( (SWORD32)Breg >= (SWORD32)Areg ) || 
							( (SWORD32)Breg < (SWORD32)(- Areg ) )) ) {
							SET_FLAGS(EmulatorState_ErrorFlag);
						}
						InstCycles = 5;
						DROP();
					}
					break;

				case O_xdble: // extend to double
					InstCycles++;
					Creg = Breg;
					Breg = ((SWORD32)Areg < 0 ? -1 : 0);
					break;

				case O_csngl: // check single
					if ((((SWORD32)Areg < 0)  && ((SWORD32)Breg != -1)) || 
						(((SWORD32)Areg >= 0) && (Breg != 0))) {
						SET_FLAGS(EmulatorState_ErrorFlag);
					}
					InstCycles = 3;
					Breg = Creg;
					break;

				case O_resetch: { // reset channel 
						WORD32 OldAreg = Areg;
						// TODO replace with link objects
						// if Areg points to link channel then link hardware reset. Issue
						// notification in this case?
						Areg = myMemory->getWord(Areg);
						myMemory->setWord(OldAreg, NotProcess_p);
					}
					break;

				case O_sthf: // store high priority front pointer
					SET_FLAGS(EmulatorState_QueueInstruction);
					HiHead = POP();
					break;

				case O_stlf: // store low priority front pointer
					SET_FLAGS(EmulatorState_QueueInstruction);
					LoHead = POP();
					break;

				case O_sttimer: // store timer: the clocks are always running....
					SET_FLAGS(EmulatorState_TimerInstruction);
					HiClock = POP();
					LoClock = HiClock;
					CycleCountSinceReset = 0;
					break;

				case O_sthb: // store high priority back pointer
					SET_FLAGS(EmulatorState_QueueInstruction);
					HiTail = POP();
					break;

				case O_stlb: // store low priority back pointer
					SET_FLAGS(EmulatorState_QueueInstruction);
					LoTail = POP();
					break;

				case O_saveh: // save high priority queue registers
					myMemory->setWord(Areg, HiHead);
					myMemory->setWord(Areg + 4, HiTail);
					InstCycles = 4;
					DROP();
					break;

				case O_savel: // save low priority queue registers
					myMemory->setWord(Areg, LoHead);
					myMemory->setWord(Areg + 4, LoTail);
					InstCycles = 4;
					DROP();
					break;

				case O_clrhalterr: // Clear Halt On Error Flag
					CLEAR_FLAGS(EmulatorState_HaltOnError);
					break;

				case O_sethalterr: // Set Halt On Error Flag
					SET_FLAGS(EmulatorState_HaltOnError);
					break;

				case O_testhalterr: // Test Halt On Error Flag
					PUSH(IS_FLAG_SET(EmulatorState_HaltOnError));
					InstCycles++;
					break;

				case O_dup: // duplicate top of stack
					Creg = Breg;
					Breg = Areg;
					break;

				case O_tin: { // timer input
						SWORD32 sCurrPriClock = (SWORD32) (Wdesc_HiPriority(Wdesc) ?  HiClock : LoClock);
						SWORD32 sAreg = (SWORD32) Areg;
						if (sAreg > sCurrPriClock) {
							// process is waiting for some time in the future, so
							// sleep. Store the time that the process is waiting for
							myMemory->setWord(W_TIME(Wdesc), Areg);
						} else {
							// process is waiting for some time in the past - continue
							;
						}
					}
					break;

				case O_alt: // alt start
					// Store flag to show enabling is occurring
					myMemory->setWord(W_ALTSTATE(Wdesc), Enabling_p);
					// CWG, page 87: "If any guard is immediately ready - i.e. is a
					// SKIP guard or a channel guard on a ready channel - then this
					// location [W_ALTSTATE] is set to Ready_p to indicate that a guard
					// is ready."
					// TODO how? check encs/enbs
					InstCycles++;
					break;

				case O_talt: // timer alt start
					// Store flag to show enabling is occurring and alt time not yet set
					myMemory->setWord(W_ALTSTATE(Wdesc), Enabling_p);
					myMemory->setWord(W_TLINK(Wdesc), TimeNotSet_p);
					InstCycles = 4;
					break;

				case O_enbc: { // enable channel
						WORD32 ChanAddr;
						// If conditional in Areg == BOOL_TRUE, enable channel Breg
						if (Areg) {
							InstCycles = 7;
							ChanAddr = myMemory->getWord(Breg);
							// No process waiting on channel Breg?
							if (ChanAddr == NotProcess_p) {
								// Initiate communication on channel Breg
								myMemory->setWord(Breg, Wdesc);
							}
							// The current process is waiting on channel Breg?
							else if (ChanAddr == Wdesc) {
								// Already waiting on this channel so ignore
								; // Do nothing
	                        }
							// Another process is waiting on channel Breg?
							else {
								// Set flag to show guard is ready
								myMemory->setWord(W_ALTSTATE(Wdesc), Ready_p);
							}
						}
						Breg = Creg;
					}
					break;

				case O_enbs: // enable skip
					if (Areg) {
						// Set flag to show guard is ready
						myMemory->setWord(W_ALTSTATE(Wdesc), Ready_p);
					}
					InstCycles = 3;
					break;

				case O_enbt: { // enable timer
						WORD32 AltTimeSet;
						SWORD32 sCurrPriClock;
						SWORD32 sAltTime;
						if (Areg) {
							AltTimeSet = myMemory->getWord(W_TLINK(Wdesc));
							// Time is in Breg
							// Alt time not seen yet?
							if (AltTimeSet == TimeNotSet_p) {
								// Set 'time set' flag, and set alt time to time of
								// guard
								myMemory->setWord(W_TLINK(Wdesc), Enabling_p);
								myMemory->setWord(W_TIME(Wdesc), Breg);
							} else {
								// Alt time set?
								if (AltTimeSet == Enabling_p) {
									sCurrPriClock = (SWORD32) (Wdesc_HiPriority(Wdesc) ?
											HiClock : LoClock);
									sAltTime = (SWORD32) myMemory->getWord(W_TIME(Wdesc));
									// Alt time earlier than this guard?
									if (sAltTime < sCurrPriClock) {
										// Ignore this guard
										;
									} else {
										// Alt time later than this guard
										// Set alt time to time of this guard
										myMemory->setWord(W_TIME(Wdesc), Breg);
									}
								}
							}
						}
						Breg = Creg;
						InstCycles = 8;
					}
					break;

				case O_altwt: // alt wait
					// Set flag to show no branch has been selected yet and wait until one of the guards
					// has been selected.
					myMemory->setWord(W_TEMP(Wdesc), NoneSelected_o);
					// Are none of the guards ready?
					if (myMemory->getWord(W_ALTSTATE(Wdesc)) != Ready_p) {
						myMemory->setWord(W_ALTSTATE(Wdesc), Waiting_p);
						SET_FLAGS(EmulatorState_DescheduleRequired);
					}
					break;

				case O_taltwt: { // timer alt wait
						SWORD32 sCurrPriClock;
						SWORD32 sAltTime;
						// Set flag to show no branch has been selected yet, put alt time into the timer queue
						// and wait until one of the guards is ready.
						myMemory->setWord(W_TEMP(Wdesc), Enabling_p);
						// Are none of the guards ready?
						if ( (myMemory->getWord(W_ALTSTATE(Wdesc)) != Ready_p) &&
							(myMemory->getWord(W_TLINK(Wdesc)) != Enabling_p) ) {
							// What time is it?
							sCurrPriClock = (SWORD32) (Wdesc_HiPriority(Wdesc) ? HiClock : LoClock);
							sAltTime = (SWORD32) myMemory->getWord(W_TIME(Wdesc));
							// Is the time in the past?
							if (sAltTime < sCurrPriClock) {
								// TODO unfinished code
								logFatal("taltwt is unfinished");
								exit(-1);
							} else {
								// TODO unfinished code
								logFatal("taltwt is unfinished");
								exit(-1);
							}
						}
					}
					break;

				case O_altend: // alt end
					// Set IPtr to first instruction of branch selected
					IPtr += myMemory->getWord(W_TEMP(Wdesc));
					break;

				case O_diss: // disable skip guard
					// Offset in Areg, Flag in Breg
					if (Breg && (myMemory->getWord(W_TEMP(Wdesc)) == Ready_p)) {
						// select this branch
						myMemory->setWord(W_TEMP(Wdesc), Areg);
						Areg = BOOL_TRUE;
					}
					else {
						Areg = BOOL_FALSE;
					}
					Breg = Creg;
					InstCycles = 4;
					break;

				case O_disc: // disable channel guard
					// Offset in Areg, Flag in Breg, Channel in Creg
					// Channel Creg ready and no branch selected?
					if (Breg && (myMemory->getWord(Creg) != NotProcess_p) && 
						(myMemory->getWord(W_TEMP(Wdesc)) == Ready_p)) {
						// select this branch
						myMemory->setWord(W_TEMP(Wdesc), Areg);
						Areg = BOOL_TRUE;
					} else {
						// Channel Creg not ready or a branch already selected
   						Areg = BOOL_FALSE;
					}
					InstCycles = 8;
					break;

				case O_dist: { // disable timer guard
						SWORD32 sCurrPriClock = (SWORD32) (Wdesc_HiPriority(Wdesc) ? HiClock : LoClock);
						SWORD32 sAltTime = (SWORD32) myMemory->getWord(W_TIME(Wdesc));
						// Offset in Areg, Flag in Breg, Time in Creg
						// Time later than guards time and no branch selected
						if (Breg && (sAltTime < sCurrPriClock) &&
							(myMemory->getWord(W_TEMP(Wdesc)) == Ready_p)) {
							// Select this branch
							myMemory->setWord(W_TEMP(Wdesc), Areg);
							Areg = BOOL_TRUE;
						} else {
							// Time earlier than guards time or a branch already selected
							Areg = BOOL_TRUE;
						}
						SET_FLAGS(EmulatorState_Interrupt);
					}
					break;

				case O_fpchkerr: // check floating error
					InstCycles++;
					if (IS_FLAG_SET(EmulatorState_FErrorFlag)) {
						SET_FLAGS(EmulatorState_ErrorFlag);
					} else {
						CLEAR_FLAGS(EmulatorState_ErrorFlag);
					}
					// RoundMode := ToNearest
					break;

				case O_fptesterr: // test floating error false and clear
					// TODO? use the TN61 interpretation, returning true, as the FPU is
					// not really present in the emulator at the moment?
					PUSH(IS_FLAG_CLEAR(EmulatorState_FErrorFlag));
					InstCycles++;
					// RoundMode := ToNearest
					break;

				case O_ladd: { // long add with carry (LS bit of Creg)
						// logInfoF("ladd: Areg %08X Breg %08X Carry %08X", Areg, Breg, Creg & 0x00000001);

						WORD32 AregSign = Areg & SignBit;
						WORD32 BregSign = Breg & SignBit;
						Areg = Breg + Areg + (Creg & 0x00000001);
						WORD32 resultSign = Areg & SignBit;
						if ((BregSign == AregSign) && (BregSign != resultSign)) {
							// logInfo("ladd: carry overflow");
							SET_FLAGS(EmulatorState_ErrorFlag);
						}

						// logInfoF("ladd: result %08X\n", Areg);
						InstCycles++;
					}
					break;

				case O_lsub: { // long subtract with carry (LS bit of Creg)
						WORD32 AregSign = Areg & SignBit;
						WORD32 BregSign = Breg & SignBit;
						Areg = Breg - Areg - (Creg & 0x00000001);
						WORD32 resultSign = Areg & SignBit;
						if ((BregSign != AregSign) && (AregSign == resultSign)) {
							SET_FLAGS(EmulatorState_ErrorFlag);
						}
						InstCycles++;
					}
					break;

				case O_lsum: { // long sum with carry placed in Breg
						WORD32 result = Breg + Areg;
						WORD32 newCarry = result < Breg;
						Areg = result;
						result += (Creg & 0x00000001);
						if (result < Areg)
							newCarry = 1;
						Areg = result;
						Breg = newCarry;
						InstCycles = 3;
					}
					break;

				case O_ldiff: { // long difference with borrow placed in Breg
						WORD32 carry = Creg & 0x00000001;
						WORD32 result = Breg - Areg;
						WORD32 newCarry = result > Breg;
						Areg = result;
						result = Areg - carry;
						if (result > Areg) {
							newCarry = 1;
						}
						Areg = result;
						Breg = newCarry;
						InstCycles = 3;
					}
					break;

				case O_lmul: { // long multiply
						WORD64 MulReg = (((WORD64)Breg) * Areg) + Creg;
						InstCycles = BitsPerWord + 1;
						Breg = (WORD32) ((MulReg >> BitsPerWord) & 0xffffffff);
						Areg = (WORD32) (MulReg & 0xffffffff);
						Creg = Breg;
					}
					break;

				case O_ldiv: // long divide
					// logInfoF("ldiv: Areg %08X Breg %08X Creg %08X", Areg, Breg, Creg);
					InstCycles = BitsPerWord + 3;
					if (Areg == 0 || Creg >= Areg) {
						// logInfo("ldiv: error\n");
						DROP();
						SET_FLAGS(EmulatorState_ErrorFlag);
					} else {
						if (Creg == 0) {
							// logInfo("ldiv: quick version");
							WORD32 div = Breg / Areg;
							WORD32 rem = Breg % Areg;
							Breg = rem;
							Areg = div;
						} else {
							WORD64 CregBreg = MakeWORD64(Creg, Breg);
							// std::cout << "ldiv: WORD64 version CregBreg " << std::hex << std::setw(16) << std::setfill('0') << CregBreg << std::endl;
							Breg = (WORD32) (CregBreg % Areg);
							Areg = (WORD32) (CregBreg / Areg);
						}
						// logInfoF("ldiv result: Areg %08X Breg %08X\n", Areg, Breg);
						Creg = Breg;
					}
					break;

				case O_lshl: // long shift left
					InstCycles = Areg + 3;
					if (Areg >= (BitsPerWord << 1)) {
						logDebug("lshl: Areg >= 64");
						InstCycles = 3;
						Areg = Breg = 0;
					} else {
						// Assertion: 0 <= Areg <= 2*BitsPerWord
						WORD64 ShiftReg = MakeWORD64(Creg, Breg) << Areg;
						Areg = (WORD32) (ShiftReg & 0xffffffff);
						Breg = (WORD32) ((ShiftReg >> BitsPerWord) & 0xffffffff);
					}
					Creg = Breg;
					break;

				case O_lshr: // long shift right
					InstCycles = Areg + 3;
					if (Areg >= (BitsPerWord << 1)) {
						logDebug("lshr: Areg >= 64");
						InstCycles = 3;
						Areg = Breg = 0;
					} else {
						// Assertion: 0 <= Areg <= 2*BitsPerWord
						WORD64 ShiftReg = MakeWORD64(Creg, Breg) >> Areg;
						Areg = (WORD32) (ShiftReg & 0xffffffff);
						Breg = (WORD32) ((ShiftReg >> BitsPerWord) & 0xffffffff);
					}
					Creg = Breg;
					break;

				case O_bitcnt: { // bit count
						WORD32 count, i, highestbitset;
						for (i = count = highestbitset = 0; i < BitsPerWord; i++, Areg >>= 1) {
							count += (Areg & 1);
							if (Areg & 1) {
								highestbitset = i;
							}
						}
						Areg = Breg + count;
						Breg = Creg;
						InstCycles = highestbitset + 2;
					}
					break;

				case O_bitrevword: { // reverse bits in word
						WORD32 temp = 0;
						for (int i=0; i<32; i++) {
							temp <<= 1;
							temp |= (Areg & 1);
							Areg >>= 1;
						}
						Areg = temp;
						InstCycles = BitsPerWord + 4;
					}
					break;

				case O_bitrevnbits: { // Areg = Breg with bottom Areg bits reversed
						// logInfoF("bitrevnbits: Areg (num bits): %08X Breg (number): %08X", Areg, Breg);
						WORD32 temp = 0;
						if (Areg > BitsPerWord) {
							logWarnF("bitrevnbits: UNDEFINED BEHAVIOUR Areg: %08X", Areg);
							// With thanks to Mike Bruestle's analysis & TVS
							if (Areg >= 2 * BitsPerWord) {
								Areg = 0;
							} else {
								// Reverse...
								for (int i=0; i<32; i++) {
									temp <<= 1;
									temp |= (Breg & 1);
									Breg >>= 1;
								}
								Areg = temp << (Areg - BitsPerWord);
							}
						} else {
							if (Areg != 0)  {
								// logInfo("bitrevnbits: 1 <= Areg <= 32 bits");
								// Reverse...
								// All more significant bits are zeroed. CWG p. 68. Sec 8.2
								for (int i=0; i<Areg; i++) {
									temp <<= 1;
									temp |= (Breg & 1);
									Breg >>= 1;
								}
								Areg = temp;
							}
						}
						// logInfoF("bitrevnbits: Final Areg %08X\n", Areg);
						Breg = Creg;
						InstCycles = Areg + 4;
					}
					break;

				case O_wsubdb: // form double word subscript
					Areg += (Breg << 3);
					Breg = Creg;
					InstCycles = 3;
					break;

				// T414 only instructions
				case O_cflerr: // check single length fp infinity or NaN
					if ((Areg & 0x7FFFFFFF) == Positive_Inf ||
						(((Areg & 0x7F800000) == Positive_Inf) &&
						 ((Areg & 0x7FFFFFFF) != Positive_Inf))) {
						SET_FLAGS(EmulatorState_ErrorFlag);
					}
					break;

				case O_unpacksn: //
				case O_roundsn: //
				case O_postnormsn: //
				case O_ldinf: //
					logWarnF("Unimplemented T414 opr instruction Oreg=%08X", Oreg);
					SET_FLAGS(EmulatorState_BadInstruction);
					break;

				// T800 only instructions
				case O_move2dinit: //  initialise data for 2-dimensional block move
					// Areg contains the width of the
					// block, Breg the dest addr and Creg
					// the src addr
				case O_move2dall: //
				case O_move2dnonzero: //
				case O_move2dzero: //
				case O_crcword: //
				case O_crcbyte: //
					logWarnF("Unimplemented T800 opr instruction Oreg=%08X", Oreg);
					SET_FLAGS(EmulatorState_BadInstruction);
					break;

				case O_fmul: //
					logWarnF("Unimplemented T414/T800 opr instruction Oreg=%08X", Oreg);
					SET_FLAGS(EmulatorState_BadInstruction);
					break;


				// The unimplemented instructions...
				case O_norm: //
				case O_testpranal: //

				case O_fpdup: //
				case O_fprev: //
				case O_fpldnlsn: //
				case O_fpldnldb: //
				case O_fpldnlsni: //
				case O_fpldnldbi: //
				case O_fpstnlsn: //
				case O_fpstnldb: //
				case O_fpadd: //
				case O_fpsub: //
				case O_fpmul: //
				case O_fpdiv: //
				case O_fpremfirst: //
				case O_fpremstep: //
				case O_fpldzerosn: //
				case O_fpldzerodb: //
				case O_fpldnladdsn: //
				case O_fpldnladddb: //
				case O_fpldnlmulsn: //
				case O_fpldnlmuldb: //
				case O_fpgt: //
				case O_fpeq: //
				case O_fpordered: //
				case O_fpnan: //
				case O_fpnotfinite: //
				case O_fpint: //
				case O_fpstnli32: //
				case O_fprtoi32: //
				case O_fpi32tor32: //
				case O_fpi32tor64: //
				case O_fpb32tor64: //
					logWarnF("Unimplemented opr instruction Oreg=%08X", Oreg);
					SET_FLAGS(EmulatorState_BadInstruction);
					break;

				case O_fpentry: // floating point - entry mechanism
					switch (Areg) {
						case FP_fpuseterr: // set floating error
							SET_FLAGS(EmulatorState_FErrorFlag);
							// RoundMode := ToNearest
							break;

						case FP_fpuclrerr: // clear floating error
							CLEAR_FLAGS(EmulatorState_FErrorFlag);
							// RoundMode := ToNearest
                            break;

						// The unimplemented floating-point instructions
						case FP_fpusqrtfirst: //
						case FP_fpusqrtstep: //
						case FP_fpusqrtlast: //
						case FP_fpurp: //
						case FP_fpurm: //
						case FP_fpurz: //
						case FP_fpur32tor64: //
						case FP_fpur64tor32: //
						case FP_fpuexpdec32: //
						case FP_fpuexpinc32: //
						case FP_fpuabs: //
						case FP_fpunoround: //
						case FP_fpchki32: //
						case FP_fpuchki64: //
						case FP_fpudivby2: //
						case FP_fpumulby2: //
						case FP_fpurn: //
							logWarnF("Unimplemented FP instruction Areg=%08X", Areg);
							SET_FLAGS(EmulatorState_BadInstruction);
							break;

						default: // unknown FP instruction
							logWarnF("Unknown FP instruction Areg=%08X", Areg);
							SET_FLAGS(EmulatorState_BadInstruction);
							break;

					} // End of O_fpentry switch
					break;

				// Instructions described in "Transputer Instruction Set - Appendix" by Guy Harriman.
				// Present on all Transputers.
				case O_start:
					if (IS_FLAG_SET(EmulatorState_TVS)) {
						logInfo("start executed in TVS program");
						SET_FLAGS(EmulatorState_Terminate);
					} else {
						start();
					}
					break;
					
				case O_testlds:
					PUSH(flags);
					break;
				case O_teststs:
					flags = POP();
					break;
				case O_testhardchan:
					// Parachute software link abstraction does not
					// have visibility of this.. hardware
					// version probably will though.
				case O_testldd:
					// Parachute does not store the Dreg and
					// Ereg as 'proper' CPU registers (yet).
				case O_teststd:
				case O_testlde:
				case O_testste:
					logWarnF("Unimplemented appendix opr instruction Oreg=%08X", Oreg);
					break;

				// T805 instructions
				case O_break: // Break (swap process context)
					logInfo("*** Breakpoint (break) ***");
					InstCycles = swapContextForBreakpointInstruction() ? 9 : 11;
					break;

				case O_clrj0break: // Clear J0 break flag
					CLEAR_FLAGS(EmulatorState_J0Break);
					break;

				case O_setj0break: // Set J0 break flag
					SET_FLAGS(EmulatorState_J0Break);
					break;

				case O_testj0break: // Test J0 break flag
					PUSH(IS_FLAG_SET(EmulatorState_J0Break));
					InstCycles++;
					break;

				case O_timerdisableh:
				case O_timerdisablel:
				case O_timerenableh:
				case O_timerenablel:
					break;

				case O_ldmemstartval: // Load value of MemStart address
					PUSH(MemStart);
					break;

				case O_pop: // Pop processor stack
					Creg = POP();
					break;

				case O_lddevid: // Load device identity
					PUSH(19); // T805 uses values 10-19. I'm a late series T805.
					break;

                // Nonstandard emulator functions
				case X_togglemonitor:
					if (IS_FLAG_SET(DebugFlags_Monitor)) {
						logInfo("Exitting monitor");
						CLEAR_FLAGS(DebugFlags_Monitor);
					} else {
						logInfo("Entering monitor");
						SET_FLAGS(DebugFlags_Monitor);
					}
					break;
				case X_toggledisasm:
					if (IS_FLAG_SET(Debug_OprCodes)) {
						logInfo("Stopping disassembly");
						CLEAR_FLAGS(Debug_OprCodes);
						CLEAR_FLAGS(MemAccessDebug_ReadWriteData);
					} else {
						logInfo("Starting disassembly");
						SET_FLAGS(Debug_OprCodes);
						SET_FLAGS(MemAccessDebug_ReadWriteData);
					}
					break;
				case X_terminate:
					logDebug("Terminating emulator upon terminate instruction");
					SET_FLAGS(EmulatorState_Terminate);
					break;

				case X_marker:
					logInfo("*** MARKER ***");
					// otherwise does nothing
					break;

				case X_emuquery: {
						WORD32 response = NotProcess_p;
						switch (Areg) {
							case EQ_memtop:
								response = myMemory->getMemEnd();
								break;
							default: // unknown EQ instruction
								logWarnF("Unknown EQ instruction Areg=%08X", Areg);
								SET_FLAGS(EmulatorState_BadInstruction);
								break;
						}
						PUSH(response);
					}
					break;

				default: // unknown opr instruction
					logWarnF("Unknown opr instruction Oreg=%08X", Oreg);
					SET_FLAGS(EmulatorState_BadInstruction);
					break;

			} // End of D_opr Oreg switch
			break;

	} // End of instruction switch

	// Reset Oreg if that last one wasn't a prefix
	// TODO: add another flag that is cleared before interpretation and set
	// on pfix / nfix - this may be quicker than checking the Instruction
	// twice here.
	if ((Instruction != D_pfix) && (Instruction != D_nfix)) {
		Oreg = 0;
	}

	// TODO
	// Detect bad instructions
	if (IS_FLAG_SET(EmulatorState_BadInstruction)) {
		// TODO is this IPtr the wrong one? It's the next instruction, surely?
		logFatalF("Bad instruction: #%08X Oreg:#%08X IPtr:%08X %s", Instruction, OldOreg, IPtr,
				(Instruction == D_opr) ?
				disassembleIndirectOperation(OldOreg, Areg) :
				disassembleDirectOperation(Instruction, OldOreg)
			 );
		DumpRegs(LOGLEVEL_FATAL);
		SET_FLAGS(EmulatorState_Terminate);
	}

	// Was a low priority process interrupted by a high priority one?
	// Is a schedule required?
	if (ScheduleWdesc != NotProcess_p) {
		logDebug("Schedule required");
		if (Wdesc_HiPriority(ScheduleWdesc)) {
			// high priority, so ScheduleWdesc = WPtr
			// TODO find this in the Transputer Handbook
			myMemory->setWord(W_LINK(ScheduleWdesc), NotProcess_p);
			if (HiHead == NotProcess_p) {
				HiHead = ScheduleWdesc;
			} else {
				myMemory->setWord(W_LINK(HiTail), ScheduleWdesc);
			}
			HiTail = ScheduleWdesc;
		} else {
			// Low priority so ScheduleWdesc is not WPtr
			myMemory->setWord(W_LINK(ScheduleWdesc), NotProcess_p);
			if (LoHead == NotProcess_p) {
				LoHead = ScheduleWdesc;
			} else {
				myMemory->setWord(W_LINK(LoTail), ScheduleWdesc);
			}
			LoTail = ScheduleWdesc;
		}
	}
	SET_FLAGS(EmulatorState_QueueInstruction);
	// The instruction may have been one which can cause a deschedule while
	// waiting for another process (in, out, outword, outbyte) or it may be
	// a process control instruction which could require a deschedule. Or
	// it could be a j or lend in a low-priority task, whose quantum of
	// execution has expired. (In this latter case, DeschedulePending would
	// have been set).  In all these cases, the DescheduleRequired flag
	// will be set. Here's where we actually do that deschedule, rather
	// than duplicate this code for every deschedulable instruction...
	if (IS_FLAG_SET(EmulatorState_DescheduleRequired)) {
		logDebug("Deschedule required");
		if ((Wdesc_HiPriority(Wdesc) && (HiHead==NotProcess_p)) ||
			((!Wdesc_HiPriority(Wdesc)) && (Wdesc_WPtr(LoHead)==NotProcess_p))) {
			// Do nothing - Nothing needed to be descheduled.
            logDebug("Nothing to deschedule");
		} else {
			// Store the IPtr in the workspace
			myMemory->setWord(W_IPTR(Wdesc), IPtr);
			// Add the process to the relevant scheduling list
			if (Wdesc_HiPriority(Wdesc)) {
				// High priority => Wdesc == WPtr
				Wdesc = HiHead;
				IPtr = myMemory->getWord(W_IPTR(Wdesc));
				HiHead = myMemory->getWord(W_LINK(Wdesc));
            } else {
				// Low priority => Wdesc != Wptr
				Wdesc = LoHead;
				IPtr = myMemory->getWord(W_IPTR(Wdesc));
				LoHead = myMemory->getWord(W_LINK(Wdesc));
			}
			logDebugF("New IPtr is #%08X", IPtr);
		}
		LoClockLastQuantumExpiry = LoClock;
		SET_FLAGS(EmulatorState_QueueInstruction);
		// Flags_DescheduleRequired is cleared each time round the 
		// execute loop.
		CLEAR_FLAGS(EmulatorState_DeschedulePending);
	}

	// The instruction took InstCycles + MemCycles machine cycles 
	// to complete. During that time, the high / low priority clocks 
	// will have advanced, and if we're running a low priority process, 
	// its 2048 HiClock quantum may have expired, and therefore it is
	// a candidate for descheduling the next time a j or lend
	// instruction is encountered. (i.e. DeschedulePending is set)
	MemCycles = myMemory->getCurrentCyclesAndReset();

	// So, let time pass for the clocks and quantum expiry timer...  Using
	// the number of clock cycles since the startup, or the last sttimer
	// instruction... 
	CycleCount += InstCycles + MemCycles;
	CycleCountSinceReset += InstCycles + MemCycles;
	HiClock = CycleCountSinceReset / 20;
	LoClock = HiClock / 64;

	// Check quantum expiry. If we're running a low priority 
	// process, check to see if it has had its quantum, and 
	// if so, set the DeschedulePending flag.
	if (! Wdesc_HiPriority(Wdesc)) {
		if (LoClock >= (LoClockLastQuantumExpiry + MaxQuantum)) {
			SET_FLAGS((EmulatorState_DeschedulePending|EmulatorState_TimerInstruction));
			logDebug("Quantum expired; requesting deschedule");
			LoClockLastQuantumExpiry = LoClock;
		}
	}

	// Now dump out registers if we're not dealing with a prefix,
	// and the debug level is high enough. InstructionStartIPtr
	// means that if nfix and pfix instructions are being debugged, the
	// correct value for IPtr gets output... 
	if (Instruction != D_pfix && Instruction != D_nfix) {
		InstructionStartIPtr = IPtr;
		if ((flags & DebugFlags_DebugLevel) >= Debug_DisRegs) {
			DumpRegs(LOGLEVEL_DEBUG);
			if (IS_FLAG_SET(EmulatorState_QueueInstruction))
				DumpQueueRegs(LOGLEVEL_DEBUG);
			if (IS_FLAG_SET(EmulatorState_TimerInstruction))
				DumpClockRegs(LOGLEVEL_DEBUG, InstCycles + MemCycles);
			if (IS_FLAG_SET(DebugFlags_eForth))
				DumpeForthDiagnostics(LOGLEVEL_DEBUG);
		}
	}

	// Halt-On-Error and Error flags => terminate
	if ((flags & (EmulatorState_ErrorFlag | EmulatorState_HaltOnError)) ==
		       (EmulatorState_ErrorFlag | EmulatorState_HaltOnError)) {
		SET_FLAGS(EmulatorState_Terminate);
		logWarn("Halt-On-Error and Error set. Stopping.");
	}
	if (IS_FLAG_SET(DebugFlags_DebugLevel | DebugFlags_Monitor)) {
		logFormat(LOGLEVEL_DEBUG, "");
	}
}


// See TTH, p53
// Transputer Instruction Set - Appendix states that the first link to receive a
// byte handles the boot/peek/poke protocol, and that the links are polled in a
// repeating cycle starting at link 0.
// Note that Parachute is not a microcode emulator, and does not have a reset or
// analyse 'pin'.
void CPU::bootFromLink0() {
	bootLen = 0;
	Link *bootLink = NULL;
	int linkNo = -1;
	Areg = IPtr;
	Breg = Wdesc;
	IPtr = MemStart;
	// Boot from the link in Creg
	switch (Creg) {
		case Link0Input: linkNo = 0; break;
		case Link1Input: linkNo = 1; break;
		case Link2Input: linkNo = 2; break;
		case Link3Input: linkNo = 3; break;
		default:
			 logFatal("Creg is not set to a valid link input address for boot");
			 exit(1);
	}
	bootLink = myLinks[linkNo];
	// Repeatedly read first byte:
	// 'poke': 0 => read address word, data word, store data word at address
	// 'peek': 1 => read word, output word at that address
	// 'boot': x where x>1, x is the length of boot code to read into MemStart onwards
	BYTE8 ctrl = 0;
	do {
		try {
			ctrl = bootLink->readByte();
			switch (ctrl) {
				case 1: // peek
					try {
						WORD32 addr = bootLink->readWord();
						WORD32 value = 0xDEADF00D;
						if (myMemory->isLegalMemory(addr)) {
							value = myMemory->getWord(addr);
						} else {
							logWarnF("Boot-peek requested read from bad address %08X", addr);
						}
						if (IS_FLAG_SET(DebugFlags_LinkComms)) {
							logDebugF("Boot-peek @ %08X = %08X", addr, value);
						}
						bootLink->writeWord(value);
					} catch (exception &e) {
						logFatalF("I/O failure on link %d during boot-peek: %s", linkNo, e.what());
						exit(1);
					}
					break;
				case 0: // poke
					try {
						WORD32 addr = bootLink->readWord();
						WORD32 value = bootLink->readWord();
						if (myMemory->isLegalMemory(addr)) {
							myMemory->setWord(addr, value);
						} else {
							logWarnF("Boot-poke requested write to bad address %08X value %08X", addr, value);
						}
						if (IS_FLAG_SET(DebugFlags_LinkComms)) {
							logDebugF("Boot-poke stored %08X @ %08X", value,  addr);
						}
					} catch (exception &e) {
						logFatalF("I/O failure on link %d during boot-poke: %s", linkNo, e.what());
						exit(1);
					}
					break;
				default: // boot
					bootLen = ctrl;
					try {
						if (IS_FLAG_SET(DebugFlags_LinkComms)) {
							logDebugF("Primary bootstrap contains 0x%02X bytes", bootLen);
						}
						WORD32 addr = MemStart;
						for (int i = 0; i < ctrl; i++) {
							BYTE8 value = bootLink->readByte();
							// addr is going to be valid, always. There's always at least
							// 0xff bytes of memory after MemStart.
							myMemory->setByte(addr++, value);
						}
					} catch (exception &e) {
						logFatalF("I/O failure on link %d during bootstrap: %s", linkNo, e.what());
						exit(1);
					}
					break;
			}
		} catch (exception &e) {
			logFatalF("Boot failed to read control byte from link %d: %s", linkNo, e.what());
			exit(1);
		}
	} while (ctrl <= 1);
}

void CPU::emulate(const bool bootFromROM) {
	myBootFromROM = bootFromROM;
	// Initialise timing subsystem
	CycleCount = CycleCountSinceReset = 
		HiClock = LoClock = LoClockLastQuantumExpiry = 0L;
	// Initialise T800 registers. See TTS, p30, CWG p74
	Oreg = Areg = Breg = 0;
	FAreg = FBreg = FCreg = (REAL64)0.0;
	// These 'preserved bits' don't tie up with those given in the spec of
	// the start instruction, in Transputer Instruction Set - Appendix.
	// That does not include the descheduling bits, FErrorFlag, and does
	// include the EnableJ0Break flag.
	flags = flags & (~(EmulatorState_ErrorFlag | EmulatorState_FErrorFlag |
						EmulatorState_HaltOnError |
						EmulatorState_DeschedulePending |
						EmulatorState_DescheduleRequired));
	// Set queue pointers to magic values
	HiHead = HiTail = LoHead = LoTail = 0xDEADF00D;
	// Initialise monitor
	CurrDataAddress = CurrDisasmAddress = MemStart;
	CurrDataLen = CurrDisasmLen = 64;
	LastAjwInBytes = 16; // useful, perhaps, until next ajw

	start();

	// Go...
	//
	InstructionStartIPtr = IPtr;
	if ((flags & DebugFlags_DebugLevel) >= Debug_DisRegs) {
		DumpRegs(LOGLEVEL_DEBUG);
	}
	if ((flags & DebugFlags_Queues) == DebugFlags_Queues) {
		DumpQueueRegs(LOGLEVEL_DEBUG);
	}
	if ((flags & DebugFlags_Clocks) == DebugFlags_Clocks) {
		DumpClockRegs(LOGLEVEL_DEBUG, (WORD32)0);
	}
	// For speed, what flags do we turn on before interpreting each instruction?
	InterpFlagSet = 0;
	if (IS_FLAG_SET(DebugFlags_Clocks))
		InterpFlagSet |= EmulatorState_TimerInstruction;
	if (IS_FLAG_SET(DebugFlags_Queues))
		InterpFlagSet |= EmulatorState_QueueInstruction;

	logDebug("---- Starting Emulation ----");
	while (IS_FLAG_CLEAR(EmulatorState_Terminate)) {
		interpret();
	}
	logDebug("---- Ending Emulation ----");
	if ((flags & DebugFlags_DebugLevel) >= Debug_DisRegs) {
		DumpRegs(LOGLEVEL_DEBUG);
	}
	if ((flags & DebugFlags_Queues) == DebugFlags_Queues) {
		DumpQueueRegs(LOGLEVEL_DEBUG);
	}
	if ((flags & DebugFlags_Clocks) == DebugFlags_Clocks) {
		DumpClockRegs(LOGLEVEL_DEBUG, (WORD32)0);
	}
}

// Executed from emulate, above, and also on receipt of a start instruction.
void CPU::start() {
	if (myBootFromROM) {
		logDebug("---- Starting Boot from ROM ----");
		Areg = IPtr;
		Breg = Wdesc;
		IPtr = ResetCode;
		Wdesc = MemStart;
		Creg = 0xDEADF00D; // CWG states 'undefined'.
	} else {
		logDebug("---- Starting Boot from Link 0 ----");
		// NB: CWG states Areg is set to the previous value of IPtr, Breg the previous of Wdesc,
		// Creg a pointer to the link the Transputer booted from.
		IPtr = MemStart;
		Creg = Link0Input; // The default IServer link input
		bootFromLink0();
		// The initial workspace is the first free word of memory. A low priority process.
		Wdesc = WordAlign((WORD32)(IPtr + (WORD32)bootLen));
	}
	Wdesc |= 0x1;
}


// Evaluation stack routines
// Use like: HiWord=POP(); (HiWord=TOS, shuffle)
inline void CPU::DROP(void) {
	Areg = Breg; Breg = Creg;
}

inline WORD32 CPU::POP(void) {
	WORD32 r = Areg;
	DROP();
	return r;
}

inline void CPU::PUSH(WORD32 x) {
	Creg = Breg; Breg = Areg; Areg = x;
}
