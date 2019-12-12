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
    Stream *initStdin() {
        std::streambuf *buf = std::cin.rdbuf();
        auto *stdinStream = new ConsoleStream(buf);
        stdinStream->isWritable = false;
        stdinStream->isReadable = true;
        return stdinStream;
    }
    Stream *initStdout() {
        std::streambuf *buf = std::cout.rdbuf();
        auto *stdoutStream = new ConsoleStream(buf);
        stdoutStream->isWritable = true;
        stdoutStream->isReadable = false;
        return stdoutStream;
    }
    Stream *initStderr() {
        std::streambuf *buf = std::cerr.rdbuf();
        auto *stderrStream = new ConsoleStream(buf);
        stderrStream->isWritable = true;
        stderrStream->isReadable = false;
        return stderrStream;
    }
}

void ConsoleStream::_setStreamBuf(std::streambuf *buffer) {
    iostream.rdbuf(buffer);
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
        myFiles[i] = nullptr;
    }

    myNextAvailableFile = FILE_STDERR + 1;
}

Platform::~Platform() {
    logDebug("Destroying base platform");

    // Close all open streams
    for (int i=0; i < MAX_FILES; i++) {
        if (myFiles[i] != nullptr) {
            logDebugF("Stream #%d has been allocated", i);
            if (myFiles[i]->is_open()) {
                logDebugF("Closing open streams #%d", i);
                myFiles[i]->close();
            }
        }
    }
}

void Platform::setDebug(bool newDebug) {
    bDebug = newDebug;
}

void Platform::writeStream(int streamId, WORD16 size, BYTE8 *buffer) throw (std::exception) {
    if (streamId < 0 || streamId >= MAX_FILES) {
        logWarnF("Attempt to write to out-of-range stream id #%d", streamId);
        throw std::range_error("Stream id out of range");
    }
    StreamPtr pStream = myFiles[streamId];
    if (pStream == nullptr) {
        logWarnF("Attempt to write to unopen stream #%d", streamId);
        throw std::invalid_argument("Stream id not open");
    }
    if (! pStream->isWritable) {
        logWarnF("Attempt to write to non-writable stream #%d", streamId);
        throw std::runtime_error("Stream not writable");
    }
    pStream->write(size, buffer);
}

void Platform::readStream(int streamId, WORD16 size, BYTE8 *buffer) throw (std::exception) {
    if (streamId < 0 || streamId >= MAX_FILES) {
        logWarnF("Attempt to read from out-of-range stream id #%d", streamId);
        throw std::range_error("Stream id out of range");
    }
    StreamPtr pStream = myFiles[streamId];
    if (pStream == nullptr) {
        logWarnF("Attempt to read from unopen stream #%d", streamId);
        throw std::invalid_argument("Stream id not open");
    }
    if (! pStream->isReadable) {
        logWarnF("Attempt to read from non-readable stream #%d", streamId);
        throw std::runtime_error("Stream not readable");
    }
    pStream->read(size, buffer);
}

// For use by tests...
void Platform::_setStreamBuf(const int streamId, std::streambuf *buffer) {
    StreamPtr pStream = myFiles[streamId];
    ConsoleStream * pConsoleStream = dynamic_cast<ConsoleStream *>(pStream);
    pConsoleStream->_setStreamBuf(buffer);
}
