//------------------------------------------------------------------------------
//
// File        : platform.h
// Description : Abstract base class for Console, Time
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 05/03/2019
//
// (C) 2005-2020 Matt J. Gumbley
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
#include <memory>

#include <log.h>

#include "platformdetection.h"
#include "types.h"

#ifdef PLATFORM_WINDOWS
// For MSVC, shut up about throw (exception)
#pragma warning( disable : 4290 )
#endif

class UTCTime {
public:
    UTCTime() {
        myDayOfMonth = 0;
        myMonthOfYear = 0;
        myYear = 0;
        myHour = 0;
        myMinute = 0;
        mySecond = 0;
        myMillisecond = 0;
    }

    UTCTime(int const dayOfMonth, int const monthOfYear, int const year,
            int const hour, int const minute, int const second, int const millisecond):
            myDayOfMonth(dayOfMonth), myMonthOfYear(monthOfYear), myYear(year),
            myHour(hour), myMinute(minute), mySecond(second), myMillisecond(millisecond) {}

    UTCTime &operator=(const UTCTime & other) = default;

    int myDayOfMonth;  // day of month, (1 .. 31)
    int myMonthOfYear; // month of year, (1 .. 12)
    int myYear;        // year, (1900 .. )
    int myHour;        // hour, (0 .. 23)
    int myMinute;      // minute, (0 .. 59)
    int mySecond;      // second, (0 .. 59)
    int myMillisecond; // millisecond, (0 .. 999)
};

const int MAX_FILES = 128;
const int FILE_STDIN = 0;
const int FILE_STDOUT = 1;
const int FILE_STDERR = 2;

class Stream;

class Platform {
public:
    Platform();
    virtual void initialise() noexcept(false) = 0;
    virtual ~Platform();
    void setDebug(bool newDebug);

    virtual bool isConsoleCharAvailable() = 0;
    virtual BYTE8 getConsoleChar() = 0;
    virtual void putConsoleChar(BYTE8 ch) = 0;

    virtual WORD32 getTimeMillis() = 0;
    virtual UTCTime getUTCTime() = 0;

    // The WORD16s used for size parameters come from the protocol definition of REQ_READ, REQ_WRITE.
    WORD16 writeStream(int streamId, WORD16 size, BYTE8* buffer) noexcept(false);
    WORD16 readStream(int streamId, WORD16 size, BYTE8* buffer) noexcept(false);

    // For use by tests...
    void _setStreamBuf(int streamId, std::streambuf *buffer);

protected:
    bool bDebug;
    std::unique_ptr<Stream> myFiles[MAX_FILES];
    int myNextAvailableFile;
};

#endif // _PLATFORM_H

