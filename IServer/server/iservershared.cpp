//------------------------------------------------------------------------------
//
// File        : iservershared.cpp
// Description : Shared data and code between the IServer and EmuServer.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 07/07/2026
//
// (C) 2005-2026 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <cstdio>
#include <string>
using namespace std;

#include "iservershared.h"
#include "log.h"
#include "link.h"
#include "linkfactory.h"
#include "inmemorylink.h"
#include "platform.h"
#include "platformfactory.h"
#include "misc.h"
#include "filesystem.h"
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


char currentPath[FILENAME_MAX];
std::string myRootDirectory;
Platform *myPlatform;
PlatformFactory *platformFactory;
Link *myLink;
LinkFactory *linkFactory;
InMemoryLinkFactory *inMemoryLinkFactory;
std::string bootFile;
bool debugPlatform;
bool debugProtocol;
bool debugLink;
bool debugLinkRaw;
bool monitorLink;
std::string fullCommandLine;
std::string programCommandLine;
bool finished;


void setupCurrentPathAndRootDirectory() {
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

}

bool fileExists(const std::string &filename)
{
    return access(filename.c_str(), 0) == 0;
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

// Send a file's contents over the link, typically a boot file.
// A boot file must start with a byte indicating its length; if the code is longer than 255 bytes, the boot file
// must contain a chain loader, first.
// Precondition: sendFile != empty
void sendFileOverLink(std::string sendFile, std::string fileDescription) {
	// Open file and set exceptions to be thrown on failure
	ifstream fileStream;
	fileStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try {
		BYTE8 buf[128];
		streamsize nread = 0;
		fileStream.open(sendFile, ifstream::in | ifstream::binary);
		do {
			fileStream.exceptions(std::ifstream::goodbit);
			fileStream.read(reinterpret_cast<char *>(buf), 128);
			nread = fileStream.gcount();
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
						fileStream.close();
						finished = true;
						return;
					}
				}
			}
		} while (nread > 0);
	} catch (std::system_error& e) {
		logFatalF("Could not open %s file %s: %s", fileDescription.c_str(), sendFile.c_str(), e.code().message().c_str());
		finished = true;
	}
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
	if (inMemoryLinkFactory != NULL) {
		delete inMemoryLinkFactory;
	}
	fflush(stdout);
}
