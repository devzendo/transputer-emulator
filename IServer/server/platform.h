//------------------------------------------------------------------------------
//
// File        : platform.h
// Description : Abstract base class for Console, Time
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 05/03/2019
//
// (C) 2005-2019 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _PLATFORM_H
#define _PLATFORM_H

#include <exception>
#include <ios>
#include <iostream>
#include <fstream>

#include "platformdetection.h"
#include "types.h"

#ifdef PLATFORM_WINDOWS
// For MSVC, shut up about throw (exception)
#pragma warning( disable : 4290 )
#endif

class UTCTime {
public:
    UTCTime(int const dayOfMonth, int const monthOfYear, int const year,
            int const hour, int const minute, int const second, int const millisecond):
            myDayOfMonth(dayOfMonth), myMonthOfYear(monthOfYear), myYear(year),
            myHour(hour), myMinute(minute), mySecond(second), myMillisecond(millisecond) {}

    int const myDayOfMonth;  // day of month, (1 .. 31)
    int const myMonthOfYear; // month of year, (1 .. 12)
    int const myYear;        // year, (1900 .. )
    int const myHour;        // hour, (0 .. 23)
    int const myMinute;      // minute, (0 .. 59)
    int const mySecond;      // second, (0 .. 59)
    int const myMillisecond; // millisecond, (0 .. 999)
};

const int MAX_FILES = 128;
const int FILE_STDIN = 0;
const int FILE_STDOUT = 1;
const int FILE_STDERR = 2;

class Stream {
public:
    virtual bool is_open() = 0;
    virtual void close() = 0;
    virtual void write(WORD16 size, BYTE8 *buffer) = 0;
    virtual void read(WORD16 size, BYTE8 *buffer) = 0;
    bool isReadable;
    bool isWritable;
};

class FileStream: public Stream {
public:
    bool is_open() override {
        return fstream.is_open();
    };

    void close() override {
        fstream.close();
    };

    void write(WORD16 size, BYTE8 *buffer) override {
        fstream.write(reinterpret_cast<const char *>(buffer), size);
    }

    void read(WORD16 size, BYTE8 *buffer) override {
        fstream.read(reinterpret_cast<char *>(buffer), size);
    }

private:
    std::fstream fstream;
};

class ConsoleStream: public Stream {
public:
    explicit ConsoleStream(std::streambuf *buf): iostream(buf) {
    }

    bool is_open() override {
        return true;
    };

    void close() override {
        // no-op
    };

    void write(WORD16 size, BYTE8 *buffer) override {
        iostream.write(reinterpret_cast<const char *>(buffer), size);
    }

    void read(WORD16 size, BYTE8 *buffer) override {
        iostream.read(reinterpret_cast<char *>(buffer), size);
    }

    // For use by tests...
    void _setStreamBuf(std::streambuf *buffer);

private:
    std::iostream iostream;
};

// TODO change this to std::unique_ptr<Stream>
typedef Stream * StreamPtr;

class Platform {
public:
    Platform();
    virtual void initialise(void) throw (std::exception) = 0;
    virtual ~Platform(void);
    void setDebug(bool newDebug);

    virtual bool isConsoleCharAvailable() = 0;
    virtual BYTE8 getConsoleChar() = 0;
    virtual void putConsoleChar(const BYTE8 ch) = 0;

    virtual WORD32 getTimeMillis() = 0;
    virtual UTCTime getUTCTime() = 0;

    void writeStream(int streamId, WORD16 size, BYTE8* buffer) throw (std::exception);
    void readStream(int streamId, WORD16 size, BYTE8* buffer) throw (std::exception);

    // For use by tests...
    void _setStreamBuf(int streamId, std::streambuf *buffer);

protected:
    bool bDebug;
    StreamPtr myFiles[MAX_FILES];
    int myNextAvailableFile;
};

#endif // _PLATFORM_H

