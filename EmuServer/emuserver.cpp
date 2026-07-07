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
#include <io.h>
#define access _access_s
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

// global variables
static char *progName;


void usage() {
    logInfoF("Parachute v%s EmuServer " __DATE__, projectVersion);
    logInfo(" (C) 2005-2026 Matt J. Gumbley");
    logInfo("  http://devzendo.github.io/parachute");
    logInfo("Usage:");
    logInfoF("%s: [options] bootfile", progName);
    logInfo("Sends the bootfile over Link 0 using the Boot From Link facility.");
    logInfo("Link 0 is an in-memory Link between IServer and Emulator and cannot be changed.");
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
    linkFactory->singleInMemoryLink();
    // The EmuServer doesn't allow link customisation from the command line.
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

    // Start the emulator on a second thread, listening to the other side of the InMemory link.


    cleanup();

}
