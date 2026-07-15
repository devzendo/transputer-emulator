//------------------------------------------------------------------------------
//
// File        : emuserver.cpp
// Description : An emulator and IServer in one executable.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 06/07/2026
//
// (C) 2005-2026 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------


#include <fstream>
#include <cstdio>
#include <thread>

#include "constants.h"

using namespace std;

#include "platformdetection.h"
#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
#include <csignal>
#include <unistd.h>
#define GetCurrentDir getcwd
#endif
#if defined(PLATFORM_WINDOWS)
#include <direct.h>
#define GetCurrentDir _getcwd
#include <io.h>
#define access _access_s
#endif

#include "flags.h"
#include "filesystem.h"
#include "log.h"
#include "platform.h"
#include "platformfactory.h"
#include "protocolhandler.h"
#include "version.h"
#include "iservershared.h"

#include "link.h"
#include "inmemorylink.h"

#include "cpu.h"
#include "memory.h"

// global variables
static char *progName;
WORD32 flags;
long ramSize = DefaultMemSize;

bool processCommandLine(int argc, char *argv[]) {
	int newMegs = 4;
	int logLevel = LOGLEVEL_INFO;

	for (int i = 0; i < argc; i++) {
		fullCommandLine += std::string(argv[i]);
		if (i != argc-1) {
			fullCommandLine += " ";
		}
	}

	for (int i = 1; i < argc; i++) {
		logDebugF("Processing cmd line arg %d of %d : '%s'", i, argc, argv[i]);
		if (strlen(argv[i]) > 1 && argv[i][0] == '-') {
			switch (argv[i][1]) {
				default:
					if (!programCommandLine.empty()) {
						programCommandLine += " ";
					}
					programCommandLine += std::string(argv[i]);
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
								logFatal("Initial memory size must be in range [4..64] MB");
								return false;
							}
							ramSize = newMegs * Mega;
							logInfoF("Initial memory size set to #%08X (%ld) bytes", ramSize, ramSize);
						} else {
							logFatalF("'%s' is not of the form -m<number> to "
									"set the initial memory size", argv[i]);
							return false;
						}
					}
					break;
 				case 'd':
					if (strlen(argv[i]) >= 3) {
						switch (argv[i][2]) {
							case 'l':
								SET_FLAGS(DebugFlags_LinkComms);
								debugLink = true;
								break;
							case 'L':
								debugLink = true;
								debugLinkRaw = true;
								break;
							case 'p':
								debugPlatform = true;
								break;
							case 'P':
								debugProtocol = true;
								break;
							case 'a':
								SET_FLAGS(Debug_Disasm);
								break;
							case 'r':
								SET_FLAGS(Debug_DisRegs);
								break;
							case 'o':
								SET_FLAGS(Debug_OprCodes);
								break;
							case 'f': SET_FLAGS((//Debug_OprCodes |
												//MemAccessDebug_ReadWriteData | // Full is too much
												DebugFlags_LinkComms |
												DebugFlags_Clocks |
												DebugFlags_Queues |
												DebugFlags_IDiag));
								debugLink = true;
								debugLinkRaw = true;
								debugPlatform = true;
								debugProtocol = true;
								break;
							case 'i':
								SET_FLAGS(DebugFlags_IDiag);
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
								return false;
						}
					}
					break;
				case 'i':
					SET_FLAGS(DebugFlags_Monitor | Debug_DisRegs);
					break;
				case 'j':
					SET_FLAGS(EmulatorState_J0Break);
					break;
				case 'x':
					SET_FLAGS(DebugFlags_TerminateOnMemViol);
					break;
				case 'M':
					monitorLink = true;
					break;
				case '?':
				case 'h':
					usage();
					return false;
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
							return false;
					}
					setLogLevel(logLevel);
					break;
				case 'r':
					myRootDirectory = std::string(argv[i] + 2);
					break;
			}
		} else {
			if (fileExists(argv[i])) {
				bootFile = std::string(argv[i]);
			} else {
				if (!programCommandLine.empty()) {
					programCommandLine += " ";
				}
				programCommandLine += std::string(argv[i]);
			}
		}
	}
	//logDebug("End of cmd line processing");
	logDebugF("Full command line [%s]", fullCommandLine.c_str());
	logDebugF("Program command line [%s]", programCommandLine.c_str());
	return true;
}


void usage() {
    logInfoF("Parachute v%s EmuServer " __DATE__, projectVersion);
    logInfo(" (C) 2005-2026 Matt J. Gumbley");
    logInfo("  http://devzendo.github.io/parachute");
    logInfo("Usage:");
    logInfoF("%s: [options] bootfile", progName);
    logInfo("Sends the bootfile over Link 0 using the Boot From Link facility.");
    logInfo("Link 0 is an in-memory Link between IServer and Emulator and cannot be changed.");
    logInfo("Options:");
    logInfo("  -da   Enables disassembly during emulation");
    logInfo("  -dr   Enables disassembly & registers during emulation");
    logInfo("  -do   Enables disassembly & regs & opr/fpentry");
    logInfo("  -df   Full debug");
    logInfo("  -dp   Enables platform debug");
    logInfo("  -dP   Enables protocol debug");
    logInfo("  -dl   Enables link communications (high level) debug");
    logInfo("  -dL   Enables link communications (high & low level) debug");
    logInfo("  -dq   Enables queues debug");
    logInfo("  -dc   Enables clocks / timers debug");
    logInfo("  -dm   Enables memory read/write debug for data");
    logInfo("  -dM   Enables memory read/write debug for data & instructions");
    logInfo("  -m    Monitors boot link instead of handling protocol");
	logInfo("  -m<X> Sets initial memory size to X MB");
    logInfo("  -h    Displays this usage summary");
    logInfo("  -l<X> Sets log level. X is one of [diwef] for DEBUG, INFO");
    logInfo("        WARN, ERROR or FATAL. Default is INFO");
    logInfo("  -r<directory> Sets the root directory served by the IServer. Current directory if not given.");
    logInfo("Any options not understood by the EmuServer are stored to be made available to the Emulator.");
}

int main(int argc, char *argv[]) {
    progName = argv[0];
    bootFile = "";
    debugPlatform = false;
    debugProtocol = false;
    debugLink = false;
    debugLinkRaw = false;
    monitorLink = false;
    finished = false;
    int exitCode = 0;
	flags = 0;

    if (!processCommandLine(argc, argv)) {
        cleanup();
        exit(1);
    }

    setupCurrentPathAndRootDirectory();

    platformFactory = new PlatformFactory(debugPlatform);
    myPlatform = platformFactory->createPlatform();
    myPlatform->setCommandLines(fullCommandLine, programCommandLine);
    try {
        myPlatform->initialise();
    } catch (exception &e) {
        logFatalF("Could not initialise platform: %s", e.what());
        cleanup();
        exit(1);
    }

#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
    logDebug("Setting up signal handlers");
    // TODO the signal handlers should cleanup the emulator resources too, not just the shared iserver ones.
    signal(SIGSEGV, segViolHandler);
    signal(SIGINT, interruptHandler);
#endif

    // The EmuServer doesn't allow link customisation from the command line. There's only a pair of InMemoryLinks
    // between IServer and Emulator.
    inMemoryLinkFactory = new InMemoryLinkFactory(1, 0);
    myLink = inMemoryLinkFactory->linkA();
    Link * cpuLink = inMemoryLinkFactory->linkB();
    // The CPU will initialise its link during the initialise call. The IServer side needs its own initialisation.
    try {
        myLink->initialise();
    } catch (exception &e) {
        logFatalF("Could not initialise IServer link 1: %s", e.what());
        cleanup();
        exit(1);
    }

    logDebug("Constructing memory...");
    auto * memory = new Memory();
    logDebug("Memory constructed");

    logDebug("Initialising memory...");
    if (!memory->initialise(ramSize)) {
        logFatal("Memory initialisation failed");
        cleanup();
        exit(1);
    }
    logDebug("Constructing CPU...");
    auto cpu = new CPU();
    logDebug("Initialising CPU");
    Link *cpuLinks[4] = { cpuLink, nullptr, nullptr, nullptr };
    if (!cpu->initialise(memory, cpuLinks)) {
        logFatal("CPU initialisation failed");
        delete cpu;
        delete memory;
        cleanup();
        exit(1);
    }

    // Start the emulator on a second thread, listening to the other side of the InMemory link.
    auto *cpuThread = new std::thread([cpu, memory, cpuLink] {
        logDebug("Start of emulation");
        cpu->emulate(false);
        fflush(stdout);
        logDebug("End of emulation");
    });

    // start iserver operations
    if (!bootFile.empty()) {
        sendFileOverLink(bootFile, "boot");
        logDebug("End of boot file send");
    }

    if (monitorLink) {
        logDebug("Monitoring boot link");
        monitorBootLink();
    } else {
        logDebug("Processing IServer protocol");
        auto * myProtocolHandler = new ProtocolHandler(*myLink, *myPlatform, myRootDirectory);
        myProtocolHandler->setDebug(debugProtocol);
        while (!finished) {
            finished = myProtocolHandler->processFrame();
        }
        exitCode = myProtocolHandler->exitCode();
        logDebugF("Received exit code %d", exitCode);
    }

    try {
        logDebug("Resetting link");
        myLink->resetLink();
    } catch (exception &e) {
        logErrorF("Could not reset link 0: %s", e.what());
    }

    logDebug("EmuServer stop");
    cpuThread->join();
    delete cpuThread;
    delete cpu;
    delete memory;

    cleanup();
    return exitCode;
}
