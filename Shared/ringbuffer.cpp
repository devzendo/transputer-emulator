//------------------------------------------------------------------------------
//
// File        : ringbuffer.cpp
// Description : Ring buffer using preallocated memory.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 24/10/2023
//
// (C) 2005-2024 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include "ringbuffer.h"

RingBuffer::RingBuffer(char* myBuffer, std::size_t myBufferSize) {
	buffer = myBuffer;
	bufferSize = myBufferSize;
	clear();
}

RingBuffer::~RingBuffer() {
}

void RingBuffer::clear() {
	headIndex = 1;
	tailIndex = contentsSize = 0;
}

char RingBuffer::pop() {
	if (contentsSize == 0) {
		return '\0';
	}
	char front = buffer[headIndex];
	headIndex++;
	contentsSize--;
	if (headIndex == bufferSize) {
		headIndex = 0;
	}
	return front;
}

void RingBuffer::push(char newChar) {
	tailIndex++;
	contentsSize++;
	if (tailIndex == bufferSize) {
		tailIndex = 0;
	}
	if (contentsSize > bufferSize) {
		if (contentsSize != 0) {
			headIndex++;
			contentsSize--;
			if (headIndex == bufferSize) {
				headIndex = 0;
			}
		}
	}
	buffer[tailIndex] = newChar;
}


