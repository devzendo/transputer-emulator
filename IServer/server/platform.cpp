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
#include "platform.h"
#include "log.h"

namespace {
    Stream *initStdin() {
        std::streambuf *buf = std::cin.rdbuf();
        auto *stdinStream = new ConsoleStream(FILE_STDIN, buf);
        stdinStream->isWritable = false;
        stdinStream->isReadable = true;
        return stdinStream;
    }
    Stream *initStdout() {
        std::streambuf *buf = std::cout.rdbuf();
        auto *stdoutStream = new ConsoleStream(FILE_STDOUT, buf);
        stdoutStream->isWritable = true;
        stdoutStream->isReadable = false;
        return stdoutStream;
    }
    Stream *initStderr() {
        std::streambuf *buf = std::cerr.rdbuf();
        auto *stderrStream = new ConsoleStream(FILE_STDERR, buf);
        stderrStream->isWritable = true;
        stderrStream->isReadable = false;
        return stderrStream;
    }
}

Stream::Stream(const int _streamId) {
    streamId = _streamId;
}

WORD16 Stream::write(WORD16 size, BYTE8 *buffer) {
    std::iostream & stream = getIOStream();
    WORD16 written = stream.rdbuf()->sputn(reinterpret_cast<const char *>(buffer), size);
    // Amazingly, you can't find out how many bytes you've actually written using
    // stream.write(reinterpret_cast<const char *>(buffer), size);
    // There's an approach using tellp() to find out how much we actually wrote, shown at
    // https://stackoverflow.com/questions/14238572/how-many-bytes-actually-written-by-ostreamwrite
    // .. but I couldn't get it to work. So, here I'm getting at the underlying stream buffer and writing to it
    // with sputn, which _does_ give you the amount written - which is what the underlying library does, without
    // throwing the amount away. Sheesh, C++!
    if (written != size) {
        logWarnF("Failed to write %d bytes from stream #%d, wrote %d bytes instead", size, streamId, written);
        stream.setstate(std::ios::badbit);
    }
    return written;
}

// The WORD16s used for size parameters come from the protocol definition of REQ_READ, REQ_WRITE.
WORD16 Stream::read(WORD16 size, BYTE8 *buffer) {
    std::iostream & stream = getIOStream();
    // See comment above in write(..) about using the rdbuf
    WORD16 read = stream.rdbuf()->sgetn(reinterpret_cast<char *>(buffer), size);
    if (read != size) {
        logWarnF("Failed to read %d bytes from stream #%d, read %d bytes instead", size, streamId, read);
        stream.setstate(std::ios::badbit);
    }
    return read;
}

void ConsoleStream::_setStreamBuf(std::streambuf *buffer) {
    logDebugF("Setting the ConsoleStream's iostream with rdbuf with %d chars available", buffer->in_avail());
    iostream.rdbuf(buffer);
}

Platform::Platform() {
    bDebug = false;

    logDebug("Constructing platform");
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
    logDebug("Destroying platform");

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


WORD16 Platform::writeStream(int streamId, WORD16 size, BYTE8 *buffer) noexcept(false) {
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
    // TODO enforce last io op must be write - can't do this until we have open, and read.
    // Needs to be validated at the platform level first, then adapted to the protocol handler.
    return pStream->write(size, buffer);
}

WORD16 Platform::readStream(int streamId, WORD16 size, BYTE8 *buffer) noexcept(false) {
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
    // TODO enforce last io op must be read
    return pStream->read(size, buffer);
}

// For use by tests...
void Platform::_setStreamBuf(const int streamId, std::streambuf *buffer) {
    logDebugF("Setting stream buffer for stream id #%d", streamId);
    StreamPtr pStream = myFiles[streamId];
    if (pStream == nullptr) {
        logWarnF("Attempt to set stream buffer from unopen stream #%d", streamId);
        throw std::invalid_argument("Stream id not open");
    }
    if (!pStream->is_console()) {
        logWarnF("Cannot set stream buffer for a file stream stream #%d", streamId);
        throw std::runtime_error("Stream buffer cannot be set for file stream");
    }
    auto * pConsoleStream = dynamic_cast<ConsoleStream *>(pStream);
    pConsoleStream->_setStreamBuf(buffer);
}
