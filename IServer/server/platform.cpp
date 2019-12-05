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
    static void initStdin(std::iostream &stdinStream) {
        std::streambuf *buf = std::cin.rdbuf();
        stdinStream.rdbuf(buf);
    }
    static void initStdout(std::iostream &stdoutStream) {
        std::streambuf *buf = std::cout.rdbuf();
        stdoutStream.rdbuf(buf);
    }
    static void initStderr(std::iostream &stderrStream) {
        std::streambuf *buf = std::cerr.rdbuf();
        stderrStream.rdbuf(buf);
    }
}

Platform::Platform() {
    bDebug = false;
    myFiles = new std::iostream[MAX_FILES];
    for (auto &file: myFiles) {
        file = std::iostream();
    }
    // Initialise standard in, out and error
    initStdin(myFiles[FILE_STDIN]);
    initStdout(myFiles[FILE_STDOUT]);
    initStderr(myFiles[FILE_STDERR]);
    myNextAvailableFile = 3;
}

void ios(std::ios &i) {
    std::streambuf *sb = std::cout.rdbuf();
    i.is_open();
    i.close();
    i.rdbuf(sb);
}
void iostream(std::iostream &i) {
    std::streambuf *sb = std::cout.rdbuf();
    i.is_open();
    i.close();
    i.rdbuf(sb);
}
void istream(std::istream &i) {
    std::streambuf *sb = std::cout.rdbuf();
    i.is_open();
    i.close();
    i.rdbuf(sb);
}
void ostream(std::ostream &i) {
    std::streambuf *sb = std::cout.rdbuf();
    i.is_open();
    i.close();
    i.rdbuf(sb);
}
void ifstream(std::ifstream &i) {
    std::streambuf *sb = std::cout.rdbuf();
    i.is_open();
    i.close();
    i.rdbuf(sb);
}
void ofstream(std::ofstream &i) {
    std::streambuf *sb = std::cout.rdbuf();
    i.is_open();
    i.close();
    i.rdbuf(sb);
}
void fstream(std::fstream &i) {
    std::streambuf *sb = std::cout.rdbuf();
    i.is_open();
    i.close();
    i.rdbuf(sb);
}

Platform::~Platform(void) {
    logDebug("Destroying base platform");
    // Close all open files
    for (int i=0; i < MAX_FILES; i++) {
        if (myFiles[i].is_open()) {
            logDebugF("Closing open file #%d", i);
            myFiles[i].close();
        }
    }
}

void Platform::setDebug(bool newDebug) {
    bDebug = newDebug;
}
