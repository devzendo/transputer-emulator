//------------------------------------------------------------------------------
//
// File        : memstreambuf.h
// Description : Stream buffers based on an array
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 15/01/2020
// Thanks to https://tuttlem.github.io/2014/08/18/getting-istream-to-work-off-a-byte-array.html
// (C) 2005-2020 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _MEMSTREAMBUF_H
#define _MEMSTREAMBUF_H

#include <iostream>
#include "log.h"

class membuf : public std::basic_streambuf<char> {
public:
    membuf(const uint8_t *p, size_t l) {
        setg((char*)p, (char*)p, (char*)p + l);
        setp((char*)p, (char*)p + l);
    }
};

class writesensingbuf : public std::basic_streambuf<char> {
public:
    bool written = false;
    char buf = '\0';

    writesensingbuf() {
        setg((char*)&buf, (char*)&buf, (char*)&buf + 1);
        setp((char*)&buf, (char*)&buf + 1);
    }

    std::streamsize xsputn(const char* __s, std::streamsize __n) override {
        written = true;
        logWarn("Write sensing buffer was written to");
        return traits_type::eof();
    }
};

class flushsensingstreambuf : public std::basic_streambuf<char> {
public:
    bool flushed = false;
    char buf = '\0';

    flushsensingstreambuf() {
        setg((char*)&buf, (char*)&buf, (char*)&buf + 1);
        setp((char*)&buf, (char*)&buf + 1);
    }

    int sync() override {
        flushed = true;
        logWarn("Flush sensing buffer was flushed");
        return 0;
    }
};

class flushsensingfilebuf : public std::basic_filebuf<char> {
public:
    bool flushed = false;
    char buf = '\0';

    flushsensingfilebuf() {
        setg((char*)&buf, (char*)&buf, (char*)&buf + 1);
        setp((char*)&buf, (char*)&buf + 1);
    }

    int sync() override {
        flushed = true;
        logWarn("Flush sensing buffer was flushed");
        return 0;
    }
};

class memistream : public std::istream {
public:
    memistream(const uint8_t *p, size_t l) :
            std::istream(&_buffer),
            _buffer(p, l) {
        rdbuf(&_buffer);
    }

private:
    membuf _buffer;
};

class memostream : public std::ostream {
public:
    memostream(const uint8_t *p, size_t l) :
            std::ostream(&_buffer),
            _buffer(p, l) {
        rdbuf(&_buffer);
    }

private:
    membuf _buffer;
};

#endif // _MEMSTREAMBUF_H
