//------------------------------------------------------------------------------
//
// File        : testringbuffer.cpp
// Description : Tests for the ring buffer
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 24/10/2023
//
// (C) 2005-2023 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <cstdlib>
#include "gtest/gtest.h"
#include "ringbuffer.h"
#include "log.h"

static const std::size_t MEMSIZE  = 8;

class RingBufferTest : public ::testing::Test {
protected:
	void SetUp() override {
		if ((memory = (char *)malloc(MEMSIZE)) == nullptr) {
			logError("Out of memory in SetUp");
			return;
		}
		ringBuffer = new RingBuffer(memory, MEMSIZE);
	}

	void TearDown() override {
		delete ringBuffer;
		delete memory;
	}

	RingBuffer *ringBuffer = nullptr;
	char *memory = nullptr;
};


TEST_F(RingBufferTest, Start)
{
	EXPECT_NE(ringBuffer, nullptr);
}


