//------------------------------------------------------------------------------
//
// File        : iserver.cpp
// Description : main entry point for the host/file I/O server
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 19/08/2005
//
// (C) 2005-2026 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <fstream>
#include <iostream>
#include <cstring>
#include <cctype>
#include <ctime>
#include <cstdio>
#include <system_error>
using namespace std;

#include "platformdetection.h"
#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
#include <csignal>
#endif

#include "filesystem.h"
#include "log.h"
#include "link.h"
#include "linkfactory.h"
#include "misc.h"
#include "platform.h"
#include "platformfactory.h"
#include "protocolhandler.h"
#include "hexdump.h"
#include "isproto.h"
#include "version.h"
#include "iservershared.h"

// global variables, some in iservershared.
static char *progName;

bool processCommandLine(int argc, char *argv[]) {
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

				case 'M':
					monitorLink = true;
					break;
				case '?':
				case 'h':
					usage();
					return 0;
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
						case 'f':
							debugLink = true;
							debugLinkRaw = true;
							debugPlatform = true;
							debugProtocol = true;
							break;
						case 'l':
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
						default:
							usage();
							return 0;
					}
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
	return 1;
}

void usage() {
	logInfoF("Parachute v%s IServer " __DATE__, projectVersion);
	logInfo(" (C) 2005-2026 Matt J. Gumbley");
	logInfo("  http://devzendo.github.io/parachute");
	logInfo("Usage:");
	logInfoF("%s: [options] [bootfile]", progName);
	logInfo("If bootfile is specified, send this over Link 0 using the Boot From Link facility.");
	logInfo("If bootfile is not specified, start protocol handling over Link 0 immediately. This is used when");
	logInfo("booting the Emulator from ROM.");
	logInfo("Options:");
	logInfo("  -df   Full debug");
	logInfo("  -dp   Enables platform debug");
	logInfo("  -dP   Enables protocol debug");
	logInfo("  -dl   Enables link communications (high level) debug");
	logInfo("  -dL   Enables link communications (high & low level) debug");
	logInfo("  -M    Monitors boot link instead of handling protocol");
	logInfo("  -h    Displays this usage summary");
	logInfo("  -l<X> Sets log level. X is one of [diwef] for DEBUG, INFO");
	logInfo("        WARN, ERROR or FATAL. Default is INFO");
	logInfo("  -L<N><T> Sets link type. N is 0..3 and T is F, S, M, T for");
	logInfo("        FIFO, Socket, shared Memory, TTY (COM port on Windows).");
    logInfo("        Default is FIFO.");
	logInfo("        (only FIFO & TTY implemented yet)");
	logInfo("  -r<directory> Sets the root directory served by the IServer. Current directory if not given.");
	logInfo("  -T<N><TTY device file|COM number> (e.g. -T0/dev/tty.usbmodem2102 or -T013 for COM13:) for link N 0..3");
	logInfo("        (Forces link N to type T)");
	logInfo("Any options not understood by the IServer are stored to be made available to the transputer.");
}

void cleanup() {
	if (myPlatform != NULL) {
		delete myPlatform;
	}
	if (myLink != NULL) {
		delete myLink;
	}
	if (platformFactory != NULL) {
		delete platformFactory;
	}
	if (linkFactory != NULL) {
		delete linkFactory;
	}
	fflush(stdout);
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

	linkFactory = new LinkFactory(true, debugLinkRaw);
	if (!linkFactory->processCommandLine(argc, argv)) {
		logFatal("Could not process command line link arguments");
		cleanup();
		exit(1);
	}

#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
	logDebug("Setting up signal handlers");
	signal(SIGSEGV, segViolHandler);
	signal(SIGINT, interruptHandler);
#endif

	if ((myLink = linkFactory->createLink(0)) == NULL) {
		logFatal("Could not create link 0");
		cleanup();
		exit(1);
	}
	try {
		myLink->initialise();
	} catch (exception &e) {
		logFatalF("Could not initialise link 0: %s", e.what());
		cleanup();
		exit(1);
	}

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
		ProtocolHandler * myProtocolHandler = new ProtocolHandler(*myLink, *myPlatform, myRootDirectory);
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

	cleanup();
	return exitCode;
}

