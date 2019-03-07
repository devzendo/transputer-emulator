//------------------------------------------------------------------------------
//
// File        : nodeserver.cpp
// Description : main entry point for the node server
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 19/08/2005
//
// (C) 2005-2019 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include "platform.h"

#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <ctime>
#include <cerrno>
#include <fcntl.h>
using namespace std;
#include "log.h"
#include "link.h"
#include "linkfactory.h"
#include "platform.h"
#include "platformfactory.h"
#include "hexdump.h"
#include "nsproto.h"
#include "version.h"

// global variables
static char *progName;
static char *bootFile;
static bool debugPlatform;
static bool debugLink;
static bool debugLinkRaw;
static bool monitorLink;
static bool finished;
static Platform *myPlatform;
static PlatformFactory *platformFactory;
static Link *myLink;
static LinkFactory *linkFactory;
const int MSGBUF_MAX = CONSOLE_PUT_CSTR_BUF_LIMIT;
static char msgbuf[MSGBUF_MAX];

void usage() {
	logInfoF("Parachute v%s Node Server - Protocol v%d " __DATE__, projectVersion, NS_PROTOCOL_VERSION);
	logInfo(" (C) 2005-2019 Matt J. Gumbley");
	logInfo("  http://devzendo.github.io/parachute");
	logInfo("Usage:");
	logInfoF("%s: [options] [bootfile]", progName);
	logInfo("If bootfile is specified, send this over Link 0 using the Boot From Link facility.");
	logInfo("If bootfile is not specified, start protocol handling over Link 0 immediately. This is used when");
	logInfo("booting the Emulator from ROM.");
	logInfo("Options:");
	logInfo("  -df   Full debug");
    logInfo("  -dp   Enables platform debug");
	logInfo("  -dl   Enables link communications (high level) debug");
	logInfo("  -dL   Enables link communications (high & low level) debug");
	logInfo("  -m    Monitors boot link instead of handling protocol");
	logInfo("  -h    Displays this usage summary");
	logInfo("  -l<X> Sets log level. X is one of [diwef] for DEBUG, INFO");
	logInfo("        WARN, ERROR or FATAL. Default is INFO");
	logInfo("  -L<N><T> Sets link type. N is 0..3 and T is F, S, M for");
	logInfo("        FIFO, Socket or shared Memory. Default is FIFO.");
	logInfo("        (only FIFO implemented yet)");
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
						default:
							usage();
							return 0;
					}
					break;
			}
		} else {
			bootFile = argv[i];
		}
	}
	//logDebug("End of cmd line processing");
	return 1;
}

char *printableString(BYTE *orig, BYTE len) {
	for (int i = 0; i < len; i++) {
		msgbuf[i] = isprint(orig[i]) ? orig[i] : '.';
	}
	return msgbuf;
}

// Handle a single request over the link.
// Called repeatedly until we see a SERVER_EXIT request or the link fails.
// TODO: can we recover from a link failure?
void handleNodeServerProtocol(void) {
	try {
		// Read the message type identifier
		WORD32 msgType = myLink->readWord();
		if (debugLinkRaw) {
			logDebugF("Message id word is %08X", msgType);
		}
		
		switch (msgType) {
			case SERVER_GET_VERSION:
				if (debugLink) {
					logDebug("SERVER_GET_VERSION");
				}
				myLink->writeWord(NS_PROTOCOL_VERSION);
				break;
			case SERVER_EXIT:
				if (debugLink) {
					logDebug("SERVER_EXIT");
				}
				logInfo("Server is now exiting");
				finished = true;
				break;
			case CONSOLE_PUT_CHAR: {
					BYTE ch = myLink->readByte();
					if (debugLink) {
						logDebugF("CONSOLE_PUT_CHAR %02X '%c'", ch, isprint(ch) ? ch : '.');
					}
                    myPlatform->putChar(ch);
				}
				break;
			case CONSOLE_PUT_PSTR: {
					BYTE len = myLink->readByte();
					for (int i = 0; i < len; i++) {
						msgbuf[i] = myLink->readByte();
					}
					if (debugLink) {
						logDebugF("CONSOLE_PUT_PSTR len %02X", len);
					}
					for (int i = 0; i < len; i++) {
					    // TODO optimise this with a putString
						myPlatform->putChar(msgbuf[i]);
					}
				}
				break;
			case CONSOLE_PUT_CSTR: {
					WORD32 len = 0;
					for (WORD32 i = 0; true; i++) {
						BYTE b = myLink->readByte();
						if (i < MSGBUF_MAX) {
							msgbuf[i] = b;
						}
						if (b == 0) {
							len = i < MSGBUF_MAX ? i : MSGBUF_MAX;
							break;
						}
					}
					if (debugLink) {
						logDebugF("CONSOLE_PUT_CSTR len %08X", len);
					}
					for (WORD32 j = 0; j < len; j++) {
                        // TODO optimise this with a putString
                        myPlatform->putChar(msgbuf[j]);
					}
				}
				break;
			case CONSOLE_PUT_AVAILABLE:
				if (debugLink) {
					logDebug("CONSOLE_PUT_AVAILABLE");
				}
				myLink->writeByte(1); // TODO fix when we have a framebuffer
				break;
			case CONSOLE_GET_AVAILABLE: {
					if (debugLink) {
						logDebug("CONSOLE_GET_AVAILABLE");
					}
					bool ready = myPlatform->isCharAvailable();
					myLink->writeByte(ready ? 1 : 0);
				}
				break;
			case CONSOLE_GET_CHAR: {
					if (debugLink) {
						logDebug("CONSOLE_GET_CHAR");
					}
					BYTE inChar = myPlatform->getChar();
					myLink->writeByte(inChar); // TODO fix when we have a framebuffer
				}
				break;
			case TIME_GET_MILLIS: {
					if (debugLink) {
						logDebug("TIME_GET_MILLIS");
					}
					WORD32 millis = myPlatform->getTimeMillis();
					myLink->writeWord(millis);
				}
				break;
			case TIME_GET_UTC: {
					if (debugLink) {
						logDebug("TIME_GET_UTC");
					}
					UTCTime time = myPlatform->getUTCTime();
					myLink->writeWord(time.myDayOfMonth);
					myLink->writeWord(time.myMonthOfYear);
					myLink->writeWord(time.myYear);
					myLink->writeWord(time.myHour);
					myLink->writeWord(time.myMinute);
					myLink->writeWord(time.mySecond);
					myLink->writeWord(time.myMillisecond);
				}
				break;
			default:
				if ((msgType & NSS_LITLOAD) == NSS_LITLOAD) {
					int line = msgType & 0xffff;
					// deprecated this to a debug for now - should be warn
					logDebugF("LIT_LOAD_FATAL at line 0x%04X = %d", line, line);
				} else {
					logWarnF("Unknown node server protocol message type identifier %08X", msgType);
				}
				break;
		}
	} catch (exception e) {
		logFatalF("I/O failure on link 0: %s", e.what());
		finished = true;
	}
}

void monitorBootLink(void) {
	for(;;) {
		try {
			BYTE b = myLink->readByte();
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
    // open boot file
	int fd = myPlatform->open(bootFile, O_RDONLY);
	if (fd == -1) {
        char msgbuf[255];
#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
        strerror_r(errno, msgbuf, 255);
#elif defined(PLATFORM_WINDOWS)
        strerror_s(msgbuf, 255, errno);
#endif
		logFatalF("Could not open boot file '%s': %s", bootFile, msgbuf);
		finished = true;
		return;
	}
	BYTE buf[128];
	int nread;
	do {
		nread = myPlatform->read(fd, &buf, 128);
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
					myPlatform->close(fd);
					finished = true;
					return;
				}
			}
		}
	} while (nread > 0);
	myPlatform->close(fd);
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
	logWarn("Node Server interrupted. Terminating...");

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
	debugLink = false;
	debugLinkRaw = false;
	monitorLink = false;
	finished = false;

	if (!processCommandLine(argc, argv)) {
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

	// start node server operations
	if (bootFile != NULL) {
		sendBootFile();
		logDebug("End of boot file send");
	}

	if (monitorLink) {
		monitorBootLink();
	} else {
		while (!finished) {
			handleNodeServerProtocol();
		}
	}

	try {
		myLink->resetLink();
	} catch (exception &e) {
		logErrorF("Could not reset link 0: %s", e.what());
	}

	cleanup();
	return 0;
}

