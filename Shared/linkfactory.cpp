//------------------------------------------------------------------------------
//
// File        : linkfactory.cpp
// Description : Factory for creating derived classes of Link
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 19/07/2005
//
// (C) 2005-2023 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <cstdlib>
#include <cctype>
#include <cstring>
#include <fstream>
#include <iostream>
#include "platformdetection.h"
#include "types.h"
#include "constants.h"
#include "link.h"

#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
#include "fifolink.h"
#elif defined(PLATFORM_WINDOWS)
#include <windows.h>
#include "namedpipelink.h"
#endif
#include "tvslink.h"
#include "nulllink.h"

#include "linkfactory.h"
#include "log.h"

LinkFactory::LinkFactory(bool isServer, bool isDebug) {
	logDebug("LinkFactory CTOR");
	bServer = isServer;
	bDebug = isDebug;
	bTVS = false;
	for (int i = 0; i < 4; i++) {
		myLinkTypes[i] = LinkType_FIFO;
	}
}

static void linkConfigError(char *arg, const char *reason) {
	logFatalF("Command line option '%s' is not of the form -L<0..3><F|S|M> (%s)", arg, reason);
}

bool LinkFactory::processCommandLine(int argc, char *argv[]) {
	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-' && argv[i][1] == 'L') {
			if (strlen(argv[i]) != 4) {
				linkConfigError(argv[i], "not four characters long");
				return false;
			}
			char n = argv[i][2];
			char t = argv[i][3];
			if (!isdigit(n)) {
				linkConfigError(argv[i], "not -L<number>");
				return false;
			}
			if (n > '3') {
				linkConfigError(argv[i], "not in range -L<0..3>");
				return false;
			}
			if ((t != 'F' && t != 'S' && t != 'M')) {
				linkConfigError(argv[i], "type not F, S, M");
				return false;
			}
			switch (t) {
				case 'F': myLinkTypes[n - '0'] = LinkType_FIFO; break;
				case 'S': myLinkTypes[n - '0'] = LinkType_Socket; break;
				case 'M': myLinkTypes[n - '0'] = LinkType_SharedMemory; break;
			}
		}
		if (std::string(argv[i]) == "-tvs") {
			if (bServer) {
				logFatal("TVS support is only for the emulator, not iserver");
				return false;
			}
			// -tvs  program-file optional-input-file output-file
			// i     i+1          i+2                 i+3
			if (argc < i+4) {
				logFatal("-tvs requires program-file optional-input-file output-file");
				return false;
			}
			bTVS = true;
			tvsProgram = std::string(argv[++i]);
			tvsInput = std::string(argv[++i]);
			tvsOutput = std::string(argv[++i]);
			logInfoF("TVS Program [%s] Input [%s] Output [%s]", tvsProgram.c_str(), tvsInput.c_str(), tvsOutput.c_str());
			myLinkTypes[0] = LinkType_TVS;
			myLinkTypes[1] = LinkType_Null;
			myLinkTypes[2] = LinkType_Null;
			myLinkTypes[3] = LinkType_Null;
		}
	}
	return true;
}

Link *LinkFactory::createLink(int linkNo) {
	Link *newLink = NULL;
	switch (myLinkTypes[linkNo]) {
		case LinkType_FIFO:
#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
			newLink = new FIFOLink(linkNo, bServer);
#elif defined(PLATFORM_WINDOWS)
			newLink = new NamedPipeLink(linkNo, bServer);
#endif
			break;
		case LinkType_Socket:
			logFatal("Socket links not yet implemented");
			return NULL;
		case LinkType_SharedMemory:
			logFatal("Shared memory links not yet implemented");
			return NULL;
		case LinkType_TVS:
			newLink = new TVSLink(linkNo, tvsProgram, tvsInput, tvsOutput);
			break;
		case LinkType_Null:
			newLink = new NullLink(linkNo, bServer);
			break;
		default:
			logFatal("Unknown link type requested");
			return NULL;
	}
	newLink->setDebug(bDebug);
	return newLink;
}


