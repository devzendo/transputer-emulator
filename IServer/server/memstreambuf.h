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

class membuf : public std::basic_streambuf<char> {
public:
    membuf(const uint8_t *p, size_t l) {
        setg((char*)p, (char*)p, (char*)p + l);
        setp((char*)p, (char*)p + l);
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
