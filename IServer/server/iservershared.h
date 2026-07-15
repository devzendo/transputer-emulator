//------------------------------------------------------------------------------
//
// File        : iservershared.h
// Description : Shared data and code between IServer and EmuServer.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 07/07/2026
//
// (C) 2005-2026 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef ISERVERSHARED_H
#define ISERVERSHARED_H

#include "link.h"
#include "linkfactory.h"
#include "inmemorylink.h"
#include "platform.h"
#include "platformfactory.h"

extern void usage(); // different for the two programs.

extern char currentPath[FILENAME_MAX];
extern std::string myRootDirectory;
extern Platform *myPlatform;
extern PlatformFactory *platformFactory;
extern Link *myLink;
extern LinkFactory *linkFactory;
extern InMemoryLinkFactory *inMemoryLinkFactory;
extern std::string bootFile;
extern bool debugPlatform;
extern bool debugProtocol;
extern bool debugLink;
extern bool debugLinkRaw;
extern bool monitorLink;
extern std::string fullCommandLine;
extern std::string programCommandLine;
extern bool finished;

void setupCurrentPathAndRootDirectory();
bool fileExists(const std::string &filename);
#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
void segViolHandler(int sig);
void interruptHandler(int sig);
#endif
void sendFileOverLink(std::string sendFile, std::string fileDescription);
void monitorBootLink(void);



void cleanup();

#endif // ISERVERSHARED_H
