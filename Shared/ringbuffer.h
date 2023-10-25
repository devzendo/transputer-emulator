//------------------------------------------------------------------------------
//
// File        : ringbuffer.h
// Description : Ring buffer using preallocated memory.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 24/10/2023
//
// (C) 2005-2023 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _RINGBUFFER_H
#define _RINGBUFFER_H

#include <cstdlib>

class RingBuffer {
public:
	RingBuffer(char* buffer, std::size_t bufferSize);
	~RingBuffer(void);
protected:
};

#endif // _RINGBUFFER_H

