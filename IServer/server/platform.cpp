//------------------------------------------------------------------------------
//
// File        : platform.cpp
// Description : Abstract base class for Console, Timer
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 05/03/2019
//
// (C) 2005-2020 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <exception>
#include <memory>
#include <misc.h>

#include "types.h"
#include "platform.h"
#include "log.h"

enum InputOutputOperation { IO_READ, IO_WRITE, IO_NONE };

class Stream {
public:
    explicit Stream(int _streamId);
    virtual ~Stream() = default;
    virtual bool is_console() = 0;
    virtual bool is_open() = 0;
    virtual void close() = 0;
    virtual std::iostream & getIOStream() = 0;

    // The WORD16s used for size parameters come from the protocol definition of REQ_READ, REQ_WRITE.
    WORD16 write(WORD16 size, BYTE8 *buffer);
    WORD16 read(WORD16 size, BYTE8 *buffer);

    int streamId;
    bool isReadable = false;
    bool isWritable = false;
    InputOutputOperation lastIOOperation = IO_NONE;
};

class FileStream: public Stream {
public:
    explicit FileStream(int streamId, const std::string & filePath, const std::ios_base::openmode mode): Stream(streamId) {
        fstream.open(filePath, mode);
        logInfoF("Opened file %s with mode %d", filePath.c_str(), mode); // TODO does this get here if the file open fails?
        isReadable = ((mode & std::ios_base::in) != 0);
        isWritable = ((mode & std::ios_base::out) != 0);
        bool isBinary = ((mode & std::ios_base::binary) != 0);

        logInfoF("isReadable: %s, isWritable: %s, isBinary: %s",
                isReadable ? "true" : "false",
                isWritable ? "true" : "false",
                isBinary ? "true" : "false");
    }

    ~FileStream() override {
        logDebugF("Destroying file stream #%d", streamId);
    }

    bool is_console() override {
        return false;
    }

    bool is_open() override {
        return fstream.is_open();
    };

    void close() override {
        fstream.close();
    };

    std::iostream & getIOStream() override {
        return fstream;
    }

    // For use by tests...
    void _setFileBuf(std::filebuf &buffer);

private:
    std::fstream fstream;
};

class ConsoleStream: public Stream {
public:
    explicit ConsoleStream(int streamId, std::streambuf *buf) : Stream(streamId), iostream(buf) {
    }

    ~ConsoleStream() override {
        logDebugF("Destroying console stream #%d", streamId);
    };

    bool is_console() override {
        return true;
    }

    bool is_open() override {
        return true;
    };

    void close() override {
        // no-op
    };

    std::iostream & getIOStream() override {
        return iostream;
    }

    // For use by tests...
    void _setStreamBuf(std::streambuf *buffer);

private:
    std::iostream iostream;
};

namespace {
    std::unique_ptr<Stream> initStdin() {
        std::streambuf *buf = std::cin.rdbuf();
        std::unique_ptr<ConsoleStream> pStream { std::make_unique<ConsoleStream>(FILE_STDIN, buf) };
        pStream->isWritable = false;
        pStream->isReadable = true;
        return std::move(pStream);
    }
    std::unique_ptr<Stream> initStdout() {
        std::streambuf *buf = std::cout.rdbuf();
        std::unique_ptr<ConsoleStream> pStream { std::make_unique<ConsoleStream>(FILE_STDOUT, buf) };
        pStream->isWritable = true;
        pStream->isReadable = false;
        return std::move(pStream);
    }
    std::unique_ptr<Stream> initStderr() {
        std::streambuf *buf = std::cerr.rdbuf();
        std::unique_ptr<ConsoleStream> pStream { std::make_unique<ConsoleStream>(FILE_STDERR, buf) };
        pStream->isWritable = true;
        pStream->isReadable = false;
        return std::move(pStream);
    }
}

Stream::Stream(const int _streamId) {
    streamId = _streamId;
}

WORD16 Stream::write(WORD16 size, BYTE8 *buffer) {
    std::iostream & stream = getIOStream();
    // WOZERE I bet this isn't honouring text translation on Windows...

    // Original that gets the written length but doesn't honour text translation:
    // WORD16 written = stream.rdbuf()->sputn(reinterpret_cast<const char *>(buffer), size);

    // WOZERE test is expecting written to be expanded
    size_t before = stream.tellp();
    stream.write(reinterpret_cast<const char *>(buffer), size);
    WORD16 written = 0;
    if (stream.rdstate() == 0) {
        stream.flush();
        size_t after = stream.tellp();
        written = after - before;
    }
//    WORD16 written = size; // LIAR!! C++ stream abstraction is pants...

    // Amazingly, you can't find out how many bytes you've actually written using
    // stream.write(reinterpret_cast<const char *>(buffer), size);
    // There's an approach using tellp() to find out how much we actually wrote, shown at
    // https://stackoverflow.com/questions/14238572/how-many-bytes-actually-written-by-ostreamwrite
    // .. but I couldn't get it to work. So, here I'm getting at the underlying stream buffer and writing to it
    // with sputn, which _does_ give you the amount written - which is what the underlying library does, without
    // throwing the amount away. Sheesh, C++!
    if (written != size) {
        logWarnF("Failed to write %d bytes from stream #%d, wrote %d bytes instead", size, streamId, written);
//        stream.setstate(std::ios::badbit);
    } else {
        if (streamId == 1 || streamId == 2) {
            stream.flush();
        }
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

void FileStream::_setFileBuf(std::filebuf &buffer) {
    logDebugF("Setting the FileStream's fstream with rdbuf/swap with %d chars available", buffer.in_avail());
    fstream.rdbuf()->swap(buffer);
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
    std::unique_ptr<Stream> & pStream = myFiles[streamId];
    if (pStream == nullptr) {
        logWarnF("Attempt to write to unopen stream #%d", streamId);
        throw std::invalid_argument("Stream id not open");
    }
    if (! pStream->isWritable) {
        logWarnF("Attempt to write to non-writable stream #%d", streamId);
        throw std::runtime_error("Stream not writable");
    }
    logDebugF("Writing %d bytes to stream #%d", size, streamId);
    // TODO enforce last io op must be write - can't do this until we have open, and read.
    // Needs to be validated at the platform level first, then adapted to the protocol handler.
    WORD16 written = pStream->write(size, buffer);
    logDebugF("Wrote %d bytes to stream #%d", written, streamId);
    return written;
}

WORD16 Platform::readStream(int streamId, WORD16 size, BYTE8 *buffer) noexcept(false) {
    if (streamId < 0 || streamId >= MAX_FILES) {
        logWarnF("Attempt to read from out-of-range stream id #%d", streamId);
        throw std::range_error("Stream id out of range");
    }
    std::unique_ptr<Stream> & pStream = myFiles[streamId];
    if (pStream == nullptr) {
        logWarnF("Attempt to read from unopen stream #%d", streamId);
        throw std::invalid_argument("Stream id not open");
    }
    if (! pStream->isReadable) {
        logWarnF("Attempt to read from non-readable stream #%d", streamId);
        throw std::runtime_error("Stream not readable");
    }
    // TODO enforce last io op must be read
    logDebugF("Reading %d bytes to stream #%d", size, streamId);
    WORD16 read = pStream->read(size, buffer);
    logDebugF("Read %d bytes to stream #%d", read, streamId);
    return read;
}

WORD16 Platform::openFileStream(const std::string & filePath, const std::ios_base::openmode mode) {
    int streamId = 2;
    for (; streamId < MAX_FILES; streamId++) {
        if (myFiles[streamId] == nullptr) {
            break;
        }
    }
    if (streamId == MAX_FILES) {
        throw std::runtime_error(Formatter() << "No streams available to open " << filePath );
    }
    std::unique_ptr<FileStream> pStream { std::make_unique<FileStream>(streamId, filePath, mode) };
    myFiles[streamId] = std::move(pStream);
    return streamId;
}

bool Platform::closeStream(const int streamId) {
    if (streamId < 0 || streamId >= MAX_FILES) {
        logWarnF("Attempt to close out-of-range stream id #%d", streamId);
        throw std::range_error("Stream id out of range");
    }
    std::unique_ptr<Stream> & pStream = myFiles[streamId];
    if (pStream == nullptr) {
        logWarnF("Attempt to close unopen stream #%d", streamId);
        throw std::invalid_argument("Stream id not open");
    }
    logDebugF("Closing stream #%d", streamId);
    pStream->close();
    std::iostream &iostream = pStream->getIOStream();
    bool closeOk = !iostream.fail();
    logDebugF("Stream #%d %s", streamId, (closeOk ? "closed" : "failed to close correctly"));
    myFiles[streamId].reset();
    return closeOk;
}

// For use by tests...

void Platform::_setStreamBuf(const int streamId, std::streambuf *buffer) {
    logDebugF("Setting stream buffer for stream id #%d", streamId);
    std::unique_ptr<Stream> & pStream = myFiles[streamId];
    if (pStream == nullptr) {
        logWarnF("Attempt to set stream buffer from unopen stream #%d", streamId);
        throw std::invalid_argument("Stream id not open");
    }
    if (pStream->is_console()) {
        auto * pConsoleStream = dynamic_cast<ConsoleStream *>(pStream.get());
        pConsoleStream->_setStreamBuf(buffer);
    } else {
        logWarnF("Cannot set stream buffer for a FileStream stream #%d", streamId);
        throw std::runtime_error("Stream buffer cannot be set for file stream");
    }
}

void Platform::_setFileBuf(const int streamId, std::filebuf &buffer) {
    logDebugF("Setting file buffer for stream id #%d", streamId);
    std::unique_ptr<Stream> & pStream = myFiles[streamId];
    if (pStream == nullptr) {
        logWarnF("Attempt to set file buffer from unopen stream #%d", streamId);
        throw std::invalid_argument("Stream id not open");
    }
    if (pStream->is_console()) {
        logWarnF("Cannot set file buffer for a ConsoleStream stream #%d", streamId);
        throw std::runtime_error("File buffer cannot be set for console stream");
    } else {
        auto * pFileStream = dynamic_cast<FileStream *>(pStream.get());
        pFileStream->_setFileBuf(buffer);
    }
}
