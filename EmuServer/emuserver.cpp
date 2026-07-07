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

int main(int argc, char *argv[]) {
    logInfoF("Parachute v%s EmuServer " __DATE__, projectVersion);
    logInfo(" (C) 2005-2026 Matt J. Gumbley");
    logInfo("  http://devzendo.github.io/parachute");
}
