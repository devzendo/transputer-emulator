//------------------------------------------------------------------------------
//
// File        : testframecodec.cpp
// Description : Tests for encoding/decoding elements of frame buffers.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 15/11/2019
//
// (C) 2005-2019 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include "log.h"
#include "framecodec.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

class TestFrameCodec : public ::testing::Test {
protected:
    FrameCodec codec;

    void SetUp() override {
        setLogLevel(LOGLEVEL_DEBUG);
    }
};

TEST_F(TestFrameCodec, InitialFrameCounts)
{
    EXPECT_EQ(codec.getReadFrameSize(), 0L);
    EXPECT_EQ(codec.readFrameSizeOutOfRange(), true);
    EXPECT_EQ(codec.myReadFrameIndex, 0L);
    EXPECT_EQ(codec.myWriteFrameIndex, 0L);
}

TEST_F(TestFrameCodec, SetReadFrameSizeRanges)
{
    codec.setReadFrameSize(6);
    EXPECT_EQ(codec.getReadFrameSize(), 6L);
    EXPECT_EQ(codec.readFrameSizeOutOfRange(), false);

    codec.setReadFrameSize(510);
    EXPECT_EQ(codec.getReadFrameSize(), 510L);
    EXPECT_EQ(codec.readFrameSizeOutOfRange(), false);


    codec.setReadFrameSize(5);
    EXPECT_EQ(codec.getReadFrameSize(), 5L);
    EXPECT_EQ(codec.readFrameSizeOutOfRange(), true);

    codec.setReadFrameSize(511);
    EXPECT_EQ(codec.getReadFrameSize(), 511L);
    EXPECT_EQ(codec.readFrameSizeOutOfRange(), true);


}

// STRING HANDLING

// TODO test put16 indirectly
