//------------------------------------------------------------------------------
//
// File        : iserver.cpp
// Description : main entry point for the host/file I/O server
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 19/08/2005
//
// (C) 2005-2020 Matt J. Gumbley
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
#include <unistd.h>
#define GetCurrentDir getcwd
#endif
#if defined(PLATFORM_WINDOWS)
#include <direct.h>
#define GetCurrentDir _getcwd
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

// global variables
static char currentPath[FILENAME_MAX];
static char *progName;
static char *bootFile;
static bool debugPlatform;
static bool debugProtocol;
static bool debugLink;
static bool debugLinkRaw;
static bool monitorLink;
static bool finished;
static Platform *myPlatform;
static PlatformFactory *platformFactory;
static Link *myLink;
static LinkFactory *linkFactory;
static std::string myRootDirectory;

void usage() {
	logInfoF("Parachute v%s IServer" __DATE__, projectVersion);
	logInfo(" (C) 2005-2020 Matt J. Gumbley");
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
	logInfo("  -m    Monitors boot link instead of handling protocol");
	logInfo("  -h    Displays this usage summary");
	logInfo("  -l<X> Sets log level. X is one of [diwef] for DEBUG, INFO");
	logInfo("        WARN, ERROR or FATAL. Default is INFO");
	logInfo("  -L<N><T> Sets link type. N is 0..3 and T is F, S, M for");
	logInfo("        FIFO, Socket or shared Memory. Default is FIFO.");
	logInfo("        (only FIFO implemented yet)");
	logInfo("  -r<directory> Sets the root directory served by the IServer. Current directory if not given.");
}

bool processCommandLine(int argc, char *argv[]) {
	int logLevel = LOGLEVEL_INFO;
	for (int i = 1; i < argc; i++) {
		// logDebugF("Processing cmd line arg %d of %d : '%s'", i, argc, argv[i]);
		if (strlen(argv[i]) > 1 && argv[i][0] == '-') {
			switch (argv[i][1]) {
				case 'm':
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
			        char *thisArg=argv[i];
			        myRootDirectory = thisArg + 2;
			        break;
			}
		} else {
			bootFile = argv[i];
		}
	}
	//logDebug("End of cmd line processing");
	return 1;
}

void monitorBootLink(void) {
	for(;;) {
		try {
			BYTE8 b = myLink->readByte();
			//fprintf(stderr, "%c", (b != '\n') && (b != '\r') && isprint(b) ? b : '.');
			logDebugF("%02X %c", b, (isprint(b) ? b : '.'));
		} catch (exception e) {
			logFatalF("Could not read from link 0: %s", e.what());
			return;
		}
	}
}

// The boot file must start with a byte indicating its length; if the code is longer than 255 bytes, the boot file
// must contain a chain loader, first.
// Precondition: bootFile != NULL
void sendBootFile(void) {
    // Open boot file and set exceptions to be thrown on failure
    ifstream bootStream;
    bootStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        BYTE8 buf[128];
        streamsize nread = 0;
        bootStream.open(bootFile, ifstream::in | ifstream::binary);
        do {
            bootStream.exceptions(std::ifstream::goodbit);
            bootStream.read(reinterpret_cast<char *>(buf), 128);
            nread = bootStream.gcount();
            if (nread > 0) {
                if (debugLink) {
                    logDebugF("Read %d bytes of boot code; sending down link", nread);
                }
                for (int i = 0; i < nread; i++) {
                    try {
                        // TODO do this in blocks to improve performance
                        myLink->writeByte(buf[i]);
                    } catch (exception e) {
                        logFatalF("Could not write down link 0: %s", e.what());
                        bootStream.close();
                        finished = true;
                        return;
                    }
                }
            }
        } while (nread > 0);
    } catch (std::system_error& e) {
        logFatalF("Could not open boot file %s: %s", bootFile, e.code().message().c_str());
        finished = true;
        return;
    }
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

#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
void segViolHandler(int sig) {
	logFatal("Segmentation violation. Terminating");
	cleanup();
	exit(-1);
}

void interruptHandler(int sig) {
	signal(SIGINT, interruptHandler);
	logWarn("IServer interrupted. Terminating...");

	finished = true; // blocking read on link won't be interrupted...

	try {
		myLink->resetLink();
	} catch (exception &e) {
		logErrorF("Could not reset link 0: %s", e.what());
	}

	cleanup();
	exit(0);
}
#endif

int main(int argc, char *argv[]) {
	progName = argv[0];
	bootFile = NULL;
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

	// Thanks to computinglife in https://stackoverflow.com/questions/143174/how-do-i-get-the-directory-that-a-program-is-running-from
    if (!GetCurrentDir(currentPath, sizeof(currentPath))) {
        logFatalF("Could not get current working directory: %s", getLastError().c_str());
        cleanup();
        exit(1);
    }
    currentPath[sizeof(currentPath) - 1] = '\0'; /* not really required */
    if (myRootDirectory.empty()) {
        myRootDirectory = currentPath;
    }
    logDebugF("Root directory is '%s'", myRootDirectory.c_str());
    try {
        if (!pathIsDir(myRootDirectory)) {
            logFatalF("Root directory '%s' is not a directory", myRootDirectory.c_str());
            cleanup();
            exit(1);
        }
        logDebugF("Root directory '%s' is a directory.", myRootDirectory.c_str());
    } catch (exception &e) {
        logFatalF("Could not check root directory for existence: %s", e.what());
        cleanup();
        exit(1);
    }

    platformFactory = new PlatformFactory(debugPlatform);
	myPlatform = platformFactory->createPlatform();
    try {
        myPlatform->initialise();
    } catch (exception &e) {
        logFatalF("Could not initialise platform: %s", e.what());
        cleanup();
        exit(1);
    }

    linkFactory = new LinkFactory(true, debugLinkRaw);
	if (!linkFactory->processCommandLine(argc, argv)) {
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
	if (bootFile != NULL) {
		sendBootFile();
		logDebug("End of boot file send");
	}

	if (monitorLink) {
		monitorBootLink();
	} else {
	    ProtocolHandler * myProtocolHandler = new ProtocolHandler(*myLink, *myPlatform, myRootDirectory);
	    myProtocolHandler->setDebug(debugProtocol);
	    while (!finished) {
	        finished = myProtocolHandler->processFrame();
	    }
	    exitCode = myProtocolHandler->exitCode();
	}

	try {
		myLink->resetLink();
	} catch (exception &e) {
		logErrorF("Could not reset link 0: %s", e.what());
	}

	cleanup();
	return exitCode;
}

