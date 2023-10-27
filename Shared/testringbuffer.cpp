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

static const std::size_t MEMSIZE = 8;

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
	EXPECT_EQ(ringBuffer->pop(), '\0');
}

TEST_F(RingBufferTest, SingleChar)
{
	ringBuffer->push('a');
	EXPECT_EQ(ringBuffer->pop(), 'a');
	EXPECT_EQ(ringBuffer->pop(), '\0');
}

TEST_F(RingBufferTest, FillIt)
{
	ringBuffer->push('1');
	ringBuffer->push('2');
	ringBuffer->push('3');
	ringBuffer->push('4');
	ringBuffer->push('5');
	ringBuffer->push('6');
	ringBuffer->push('7');
	ringBuffer->push('8');
	EXPECT_EQ(ringBuffer->pop(), '1');
	EXPECT_EQ(ringBuffer->pop(), '2');
	EXPECT_EQ(ringBuffer->pop(), '3');
	EXPECT_EQ(ringBuffer->pop(), '4');
	EXPECT_EQ(ringBuffer->pop(), '5');
	EXPECT_EQ(ringBuffer->pop(), '6');
	EXPECT_EQ(ringBuffer->pop(), '7');
	EXPECT_EQ(ringBuffer->pop(), '8');
	EXPECT_EQ(ringBuffer->pop(), '\0');
}

TEST_F(RingBufferTest, WrapIt)
{
	ringBuffer->push('1');
	ringBuffer->push('2');
	ringBuffer->push('3');
	ringBuffer->push('4');
	ringBuffer->push('5');
	ringBuffer->push('6');
	ringBuffer->push('7');
	ringBuffer->push('8');
	ringBuffer->push('9');
	ringBuffer->push('a');
	EXPECT_EQ(ringBuffer->pop(), '3');
	EXPECT_EQ(ringBuffer->pop(), '4');
	EXPECT_EQ(ringBuffer->pop(), '5');
	EXPECT_EQ(ringBuffer->pop(), '6');
	EXPECT_EQ(ringBuffer->pop(), '7');
	EXPECT_EQ(ringBuffer->pop(), '8');
	EXPECT_EQ(ringBuffer->pop(), '9');
	EXPECT_EQ(ringBuffer->pop(), 'a');
	EXPECT_EQ(ringBuffer->pop(), '\0');
}


TEST_F(RingBufferTest, Oscillate)
{
	ringBuffer->push('3');
	EXPECT_EQ(ringBuffer->pop(), '3');
	ringBuffer->push('4');
	EXPECT_EQ(ringBuffer->pop(), '4');
	ringBuffer->push('5');
	EXPECT_EQ(ringBuffer->pop(), '5');
	ringBuffer->push('6');
	EXPECT_EQ(ringBuffer->pop(), '6');
	ringBuffer->push('7');
	EXPECT_EQ(ringBuffer->pop(), '7');
	EXPECT_EQ(ringBuffer->pop(), '\0');
}




