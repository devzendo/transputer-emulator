//------------------------------------------------------------------------------
//
// File        : platform.cpp
// Description : Abstract base class for Console, Timer
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 05/03/2019
//
// (C) 2005-2019 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <exception>

#include "types.h"
#include "constants.h"
#include "platformdetection.h"
#include "platform.h"
#include "log.h"

namespace {
    static Stream initStdin() {
        std::streambuf *buf = std::cin.rdbuf();
        ConsoleStream stdinStream(buf);
        return stdinStream;
    }
    static Stream initStdout() {
        std::streambuf *buf = std::cout.rdbuf();
        ConsoleStream stdoutStream(buf);
        return stdoutStream;
    }
    static Stream initStderr() {
        std::streambuf *buf = std::cerr.rdbuf();
        ConsoleStream stderrStream(buf);
        return stderrStream;
    }
}

Platform::Platform() {
    bDebug = false;

    // Initialise standard in, out and error
    logDebug("Initialising stdin, stdout and stderr streams");
    myFiles[FILE_STDIN] = initStdin();
    myFiles[FILE_STDOUT] = initStdout();
    myFiles[FILE_STDERR] = initStderr();

    // Now initialise all others to be unopened
    logDebug("Initialising file streams");
    for (int i=FILE_STDERR + 1; i < MAX_FILES; i++) {
        myFiles[i] = FileStream();
    }

    myNextAvailableFile = FILE_STDERR + 1;
}

Platform::~Platform(void) {
    logDebug("Destroying base platform");
    // Close all open streams
    for (int i=0; i < MAX_FILES; i++) {
        if (myFiles[i].is_open()) {
            logDebugF("Closing open streams #%d", i);
            myFiles[i].close();
        }
    }
}

void Platform::setDebug(bool newDebug) {
    bDebug = newDebug;
}
