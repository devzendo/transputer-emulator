//------------------------------------------------------------------------------
//
// File        : ringbuffer.h
// Description : Ring buffer using preallocated memory.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 24/10/2023
//
// (C) 2005-2024 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _RINGBUFFER_H
#define _RINGBUFFER_H

#include <cstdlib>

class RingBuffer {
public:
	RingBuffer(char* myBuffer, std::size_t myBufferSize);
	~RingBuffer(void);

	// Returns next character, or \0 if exhausted or empty.
	char pop();

	// Append a character to the buffer, wrapping around if needed.
	void push(char);

	// Empties the ringbuffer; resets it to initial conditions.
	void clear();

protected:
	char* buffer;
	std::size_t headIndex, tailIndex;
	std::size_t contentsSize, bufferSize;
};

#endif // _RINGBUFFER_H

