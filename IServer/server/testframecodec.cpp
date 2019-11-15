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

#include "framecodec.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

class TestFrameCodec : public ::testing::Test {
protected:
    FrameCodec codec;
};

TEST_F(TestFrameCodec, InitialFrameCounts)
{
//    EXPECT_EQ(codec.myReadFrameSize, 0L);
//    EXPECT_EQ(codec.myReadFrameIndex, 0L);
//    EXPECT_EQ(codec.myWriteFrameIndex, 0L);
}
