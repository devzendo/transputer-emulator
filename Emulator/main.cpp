//------------------------------------------------------------------------------
//
// File        : main.cpp
// Description : main entry point for the emulator
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 29/06/2005
//
// (C) 2005-2023 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <map>
#include <set>

using namespace std;

#include "platformdetection.h"
#include "constants.h"
#include "types.h"
#include "flags.h"
#include "memloc.h"
#include "memory.h"
#include "cpu.h"
#include "log.h"
#include "link.h"
#include "linkfactory.h"
#include "version.h"

// global variables
long ramSize;
long romSize;
WORD32 flags;
static char *romFile;
static char *progName;
set<WORD32> breakpointAddresses;
map<std::string, WORD32> symbolToAddress;
WORD32 SPP, RPP;

void usage() {
	logInfoF("Parachute v%s Portable Transputer Emulator " __DATE__, projectVersion);
	logInfo("  (C) 2005-2023 Matt J. Gumbley");
	logInfo("  http://devzendo.github.io/parachute");
	logInfo("Usage:");
	logInfoF("%s: [options] [romfile]", progName);
	logInfo("If romfile is given it is loaded at the end of memory, and the Emulator uses Boot From ROM. If it is not");
	logInfo("given, the Emulator uses Boot From Link, waiting for the boot protocol on Link 0.");
	logInfo("Options:");
	logInfo("  -c    Displays configuration summary");
	logInfo("  -da   Enables disassembly during emulation");
	logInfo("  -dr   Enables disassembly & registers during emulation");
	logInfo("  -do   Enables disassembly & regs & opr/fpentry");
	logInfo("  -df   Full debug");
	logInfo("  -di   Enables IServer debug");
	logInfo("  -dl   Enables link communications debug");
	logInfo("  -dq   Enables queues debug");
	logInfo("  -dc   Enables clocks / timers debug");
	logInfo("  -dm   Enables memory read/write debug for data");
	logInfo("  -dM   Enables memory read/write debug for data & instructions");
	logInfo("  -h    Displays this usage summary");
	logInfo("  -l<X> Sets log level. X is one of [diwef] for DEBUG, INFO");
	logInfo("        WARN, ERROR or FATAL. Default is INFO");
	logInfo("  -L<N><T> Sets link type. N is 0..3 and T is F, S, M for");
	logInfo("        FIFO, Socket or shared Memory. Default is FIFO.");
	logInfo("        (only FIFO implemented yet)");
	logInfo("  -m<X> Sets initial memory size to X MB");
	logInfo("  -i    Enters interactive monitor immediately");
	logInfo("  -j    Enables break on j0");
	logInfo("  -t    Terminate emulation upon memory violation");
	logInfo("  -s<F> Load a list of symbols (lines with NAME HEX-ADDRESS) from file X");
	logInfo("  -b<H> Add H (a hex address or symbol) as a breakpoint (can be repeated)");
	logInfo("        (Note: symbols must have been specified first with -s<F> to give");
	logInfo("         a symbol as a breakpoint)");
	logInfo("  -e    Enables debug features that assist eForth debugging:");
	logInfo("        Displays the data and return stacks (with symbols)");
	logInfo("        (Note: symbols must have been specified first with -s<F> and");
	logInfo("         these must include stack symbols)");
}

void showConfiguration() {
	logInfoF("Memory size:     #%08X bytes. (%dMB)", ramSize, ramSize/Mega);
	logInfoF("Internal memory: #%08X to #%08X", InternalMemStart, InternalMemEnd);
	logInfoF("External memory: #%08X to #%08X", ExternalMemStart, InternalMemStart + ramSize);
	// TODO we're not showing the ROM memory range here.
}

bool processCommandLine(int argc, char *argv[]) {
	bool showConf = false;
	int newMegs = 4;
	int logLevel = LOGLEVEL_INFO;
	for (int i = 1; i < argc; i++) {
		//logDebugF("Processing cmd line arg %d of %d : '%s'", i, argc, argv[i]);
		if (strlen(argv[i]) > 1 && argv[i][0] == '-') {
			switch (argv[i][1]) {
				case 'h':
				case '?':
				    usage();
				    return 0;
				case 'c':
					showConf = true;
					break;
				case 'm':
					if (strlen(argv[i]) >= 3) {
#if defined(PLATFORM_WINDOWS)
						int n = sscanf_s(&argv[i][2], "%d", &newMegs);
#elif defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
						int n = sscanf(&argv[i][2], "%d", &newMegs);
#endif
						if (n == 1) {
							if (newMegs < 4 || newMegs > 64) {
								logFatal("Initial memory size must be in range [1..64] MB");
								return 0;
							}
							ramSize = newMegs * Mega;
							logInfoF("Initial memory size set to #%08X (%ld) bytes", ramSize, ramSize);
						} else {
							logFatalF("'%s' is not of the form -m<number> to "
									"set the initial memory size", argv[i]);
							return 0;
						}
					} else {
						logFatal("No argument given to -m<number> to set the initial memory size");
						return 0;
					}
					break;
 				case 'l':
					switch (argv[i][2]) {
						case 'd':
							logLevel = LOGLEVEL_DEBUG;
							break;
						case 'i':
							logLevel = LOGLEVEL_INFO;
							break;
						case 'w':
							logLevel = LOGLEVEL_WARN;
							break;
						case 'e':
							logLevel = LOGLEVEL_ERROR;
							break;
						case 'f':
							logLevel = LOGLEVEL_FATAL;
							break;
						default:
							logFatal("Incorrect level given to -l<loglevel> to set logging level.");
							logFatal("<loglevel> is one of [diwef] for DEBUG, INFO, WARN, ERROR or FATAL.");
							return 0;
					}
					setLogLevel(logLevel);
					break;
 				case 'd':
					switch (argv[i][2]) {
						case 'a':
							SET_FLAGS(Debug_Disasm);
							break;
						case 'r':
							SET_FLAGS(Debug_DisRegs);
							break;
						case 'o':
							SET_FLAGS(Debug_OprCodes);
							break;
						case 'f': SET_FLAGS((Debug_OprCodes |
											MemAccessDebug_ReadWriteData | // Full is too much
											DebugFlags_LinkComms |
											DebugFlags_Clocks |
											DebugFlags_Queues |
											DebugFlags_IDiag));
							break;
						case 'i':
							SET_FLAGS(DebugFlags_IDiag);
							break;
						case 'l':
							SET_FLAGS(DebugFlags_LinkComms);
							break;
						case 'q':
							SET_FLAGS(DebugFlags_Queues);
							break;
						case 'c':
							SET_FLAGS(DebugFlags_Clocks);
							break;
						case 'm':
							SET_FLAGS(MemAccessDebug_ReadWriteData);
							break;
						case 'M':
							SET_FLAGS(MemAccessDebug_Full);
							break;
						default:
							usage();
							return 0;
					}
					break;
				case 'i':
					SET_FLAGS(DebugFlags_Monitor | Debug_DisRegs);
					break;
				case 'j':
					SET_FLAGS(EmulatorState_J0Break);
					break;
				case 't':
					SET_FLAGS(DebugFlags_TerminateOnMemViol);
					break;
				case 'b': {
					// TODO if you want a breakpoint at a symbol whose name is a valid hex number, tough!
					char symbolName[40];
					WORD32 breakpointAddress = 0;
#if defined(PLATFORM_WINDOWS)
					if (sscanf_s(&argv[i][2], "%s", symbolName) == 1) {
#elif defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
					if (sscanf(&argv[i][2], "%s", symbolName) == 1) {
#endif
						if (symbolToAddress.count(symbolName) == 1) {
							breakpointAddresses.insert(symbolToAddress[symbolName]);
							break;
						}
						// else fall through to try hex..
					}
#if defined(PLATFORM_WINDOWS)
					if (sscanf_s(&argv[i][2], "%x", &breakpointAddress) == 1) {
#elif defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
					if (sscanf(&argv[i][2], "%x", &breakpointAddress) == 1) {
#endif
						breakpointAddresses.insert(breakpointAddress);
					} else {
						logFatal("-b must be directly followed by a hex address or symbol e.g. -b8007F123");
						return 0;
					}
					}
					break;
				case 's': {
					char symbolFile[128];
#if defined(PLATFORM_WINDOWS)
					if (sscanf_s(&argv[i][2], "%s", symbolFile) == 1) {
#elif defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
					if (sscanf(&argv[i][2], "%s", symbolFile) == 1) {
#endif
						std::ifstream read(symbolFile);
						for (std::string line; std::getline(read, line); ) {
							// inserting the line into a stream that helps us parse the content
							std::stringstream ss(line);
							// read each symbol
							std::string symbolName;
							std::string addressString;
							ss >> symbolName;
							ss >> addressString;
							WORD32 symbolAddress;
#if defined(PLATFORM_WINDOWS)
							if (sscanf_s(addressString.c_str(), "08%x", &symbolAddress) == 1) {
#elif defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
							if (sscanf(addressString.c_str(), "%08x", &symbolAddress) == 1) {
#endif
								// logDebugF("name [%s] value 0x%08X", symbolName.c_str(), symbolAddress);
								symbolToAddress[symbolName] = symbolAddress;
							} else {
								logFatalF("Symbol %s 'address' %s is not a valid 8-digit hex address", symbolName.c_str(), addressString.c_str());
								return 0;
							}
						}
					} else {
                            logFatal("-s must be directly followed by a symbol file");
                            return 0;
                        }
					}
					break;
				case 'e': {
                    SET_FLAGS(DebugFlags_eForth);
                    bool gotAllSymbols =
                        (symbolToAddress.count("SPP") == 1) &&
                        (symbolToAddress.count("RPP") == 1);
                    if (!gotAllSymbols) {
                        logFatal("-e option requires SPP and RPP symbols");
                        return 0;
                    }
                    SPP = symbolToAddress["SPP"];
                    RPP = symbolToAddress["RPP"];
                    }
					break;
			}
		} else {
			romFile = argv[i];
		}
	}
	if (showConf) {
		showConfiguration();
	}
	//logDebug("End of cmd line processing");
	return 1;
}

#ifdef UNIX
void segViolHandler(int sig) {
	logFatal("Segmentation violation. Terminating");
	fflush(stdout);
	exit(-1);
}

void interruptHandler(int sig) {
	signal(SIGINT,interruptHandler);
	logWarn("Emulator interrupted.. indicating shutdown is necessary");
	fflush(stdout);
	SET_FLAGS(EmulatorState_Terminate);
}

#endif

int main(int argc, char *argv[]) {
	Memory *memory;
	CPU *cpu;
	SymbolTable *symbolTable;
	progName = argv[0];
	romFile = NULL;
	ramSize = DefaultMemSize;
	romSize = 0;
	flags = 0;

#if defined(PLATFORM_OSX) // I only use CLion on OSX
	// Stop CLion reporting this check as unreachable (which it is, on systems that don't
	// have 4 byte WORD32s).
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCDFAInspection"
#endif
	if (sizeof(WORD32) != 4) {
		logInfoF("size of WORD32 is %d bytes when it should be 4 bytes", sizeof(WORD32));
		exit(1);
	}

#if defined(PLATFORM_OSX) // I only use CLion on OSX
#pragma clang diagnostic pop
#endif

	if (!processCommandLine(argc, argv)) {
		exit(1);
	}
	LinkFactory *linkFactory = new LinkFactory(false, IS_FLAG_SET(DebugFlags_LinkComms) ? true : false);
	if (!linkFactory->processCommandLine(argc, argv)) {
		exit(1);
	}
#ifdef UNIX
	signal(SIGSEGV,segViolHandler);
	signal(SIGINT,interruptHandler);
#endif
	symbolTable = new SymbolTable();
    int symbolCount = 0;
	for (map<std::string, WORD32>::const_iterator iter = symbolToAddress.begin();
		iter != symbolToAddress.end(); iter++) {
		symbolTable->addSymbol(iter->first, iter->second);
        symbolCount++;
	}
    if (symbolCount != 0) {
        logInfoF("Added %d symbol(s)", symbolCount);
    }

	memory = new Memory();
	if (!memory->initialise(ramSize, romFile, symbolTable)) {
		delete linkFactory;
		exit(1);
	}
	cpu = new CPU();
	if (!cpu->initialise(memory, linkFactory, symbolTable)) {
		logFatal("CPU setup failed");
		delete linkFactory;
		delete memory;
		exit(1);
	}

	for (set<WORD32>::const_iterator iter = breakpointAddresses.begin();
		 iter != breakpointAddresses.end(); iter++) {
		cpu->addBreakpoint(*iter);
	}
    if (IS_FLAG_SET(DebugFlags_eForth)) {
        cpu->seteForthStackAddresses(SPP, RPP);
    }


    cpu->emulate(romFile);

	fflush(stdout);
	delete linkFactory;
	delete memory;
	delete cpu;
	return 0;
}

