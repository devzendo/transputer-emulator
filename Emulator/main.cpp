//------------------------------------------------------------------------------
//
// File        : main.cpp
// Description : main entry point for the emulator
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 29/06/2005
// Revision    : $Revision $
//
// (C) 2005 Matt J. Gumbley
// matt@gumbley.me.uk
// http://www.gumbley.me.uk/parachute
//
//------------------------------------------------------------------------------

#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
using namespace std;
#include "constants.h"
#include "types.h"
#include "flags.h"
#include "memloc.h"
#include "memory.h"
#include "cpu.h"
#include "log.h"
#include "link.h"
#include "linkfactory.h"

// global variables
long memSize;
WORD32 flags;
static char *progName;

void usage() {
	logInfoF("Parachute v%1.2f T800 Emulator " __DATE__, VERSION);
	logInfo("  (C) 2005 Matt J. Gumbley");
	logInfo("  http://www.gumbley.me.uk/parachute");
	logInfo("Usage:");
	logInfoF("%s: [options]", progName);
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
	logInfo("  -t    Terminate emulation upon memory violation");
}

void showConfiguration() {
	logInfoF("Memory size:     #%08X bytes. (%dMB)",memSize,memSize/Mega);
       	logInfoF("Internal memory: #%08X to #%08X",InternalMemStart,InternalMemEnd);
	logInfoF("External memory: #%08X to #%08X",ExternalMemStart,InternalMemStart + memSize);
}

bool processCommandLine(int argc, char *argv[]) {
	bool showConf = false;
	int newMegs = 4;
	int logLevel = LOGLEVEL_INFO;
	for (int i=1; i<argc; i++) {
		//logDebugF("Processing cmd line arg %d of %d : '%s'", i, argc, argv[i]);
		if (strlen(argv[i]) > 1 && argv[i][0] == '-') {
			switch (argv[i][1]) {
				case 'h':
				       	usage();
				       	return 0;
				case 'c':
					showConf = true;
					break;
       				case 'm':
					if (strlen(argv[i])>=3) {
        					int n = sscanf(&argv[i][2],"%d",&newMegs);
        					if (n == 1) {
							if (newMegs < 4 || newMegs > 64) {
								logFatal("Initial memory size must be in range [1..64] MB");
								return 0;
							}
							memSize = newMegs * Mega;
							logInfoF("Initial memory size set to %ld bytes", memSize);
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
							logFatal("Incorrect level given to -l<loglevel> to set logging level");
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
		        			case 'f': SET_FLAGS((Debug_OprCodes|
							MemAccessDebug_ReadWriteData| // Full is too much
							DebugFlags_LinkComms|
                             				DebugFlags_Clocks|
							DebugFlags_Queues|
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
				case 't':
		       			SET_FLAGS(DebugFlags_TerminateOnMemViol);
		       			break;

                   	}
 
		} else {
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
	progName = argv[0];
	memSize = DefaultMemSize;
	flags = 0;
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
	memory = new Memory();
	if (!memory->initialise(memSize)) {
		delete linkFactory;
		exit(1);
	}
	cpu = new CPU();
	if (!cpu->initialise(memory, linkFactory)) {
		logFatal("CPU setup failed");
		delete linkFactory;
		delete memory;
		return 0;
	}
	cpu->emulate();
	fflush(stdout);
	delete linkFactory;
	delete memory;
	delete cpu;
	return 0;
}

