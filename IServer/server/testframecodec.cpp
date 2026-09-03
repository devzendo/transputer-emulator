//------------------------------------------------------------------------------
//
// File        : testframecodec.cpp
// Description : Tests for encoding/decoding elements of frame buffers.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 15/11/2019
//
// (C) 2005-2026 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include "log.h"
#include "framecodec.h"
#include "gtest/gtest.h"

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

TEST_F(TestFrameCodec, PutGet8) {
    codec.put(static_cast<BYTE8>(0xC9));
    EXPECT_EQ(codec.myWriteFrameIndex, 1L);
    EXPECT_EQ(codec.myTransactionBuffer[0], 201);

    const BYTE8 actual = codec.get8();
    EXPECT_EQ(actual, 201);
    EXPECT_EQ(codec.myReadFrameIndex, 1L);
}

TEST_F(TestFrameCodec, Put16) {
    codec.put(static_cast<WORD16>(0xC9AF));
    EXPECT_EQ(codec.myWriteFrameIndex, 2L);
    EXPECT_EQ(codec.myTransactionBuffer[0], (BYTE8)0xAF);
    EXPECT_EQ(codec.myTransactionBuffer[1], (BYTE8)0xC9);

    const WORD16 actual = codec.get16();
    EXPECT_EQ(actual, (WORD16)0xC9AF);
    EXPECT_EQ(codec.myReadFrameIndex, 2L);
}

TEST_F(TestFrameCodec, Put32) {
    codec.put(0xAB03C9AF);
    EXPECT_EQ(codec.myWriteFrameIndex, 4L);
    EXPECT_EQ(codec.myTransactionBuffer[0], (BYTE8)0xAF);
    EXPECT_EQ(codec.myTransactionBuffer[1], (BYTE8)0xC9);
    EXPECT_EQ(codec.myTransactionBuffer[2], (BYTE8)0x03);
    EXPECT_EQ(codec.myTransactionBuffer[3], (BYTE8)0xAB);

    const WORD32 actual = codec.get32();
    EXPECT_EQ(actual, (WORD32)0xAB03C9AF);
    EXPECT_EQ(codec.myReadFrameIndex, 4L);
}

// STRING HANDLING
TEST_F(TestFrameCodec, GetEmptyString) {
    codec.put(static_cast<WORD16>(0));
    codec.put(static_cast<BYTE8>(0));

    const std::string str = codec.getString();
    EXPECT_EQ(str, "");
}

TEST_F(TestFrameCodec, GetOneCharString) {
    codec.put(static_cast<WORD16>(1));
    codec.put(static_cast<BYTE8>('A'));

    const std::string str = codec.getString();
    EXPECT_EQ(str, "A");
}

TEST_F(TestFrameCodec, GetSmallString) {
    codec.put(static_cast<WORD16>(8));
    codec.put(static_cast<BYTE8>('A'));
    codec.put(static_cast<BYTE8>('B'));
    codec.put(static_cast<BYTE8>('C'));
    codec.put(static_cast<BYTE8>('D'));
    codec.put(static_cast<BYTE8>('E'));
    codec.put(static_cast<BYTE8>('F'));
    codec.put(static_cast<BYTE8>('G'));
    codec.put(static_cast<BYTE8>('H'));
    codec.put(static_cast<BYTE8>('I')); // won't be retrieved

    const std::string str = codec.getString();
    EXPECT_EQ(str, "ABCDEFGH");
}

TEST_F(TestFrameCodec, GetMultipleStrings) {
    EXPECT_EQ(codec.myReadFrameIndex, 0);
    EXPECT_EQ(codec.myWriteFrameIndex, 0);
    codec.put(static_cast<WORD16>(3)); // 0 1
    codec.put(static_cast<BYTE8>('A')); // 2
    codec.put(static_cast<BYTE8>('B')); // 3
    codec.put(static_cast<BYTE8>('C')); // 4

    EXPECT_EQ(codec.myReadFrameIndex, 0);
    EXPECT_EQ(codec.myWriteFrameIndex, 5);
    codec.put(static_cast<WORD16>(3)); // 5 6
    codec.put(static_cast<BYTE8>('D')); // 7
    codec.put(static_cast<BYTE8>('E')); // 8
    codec.put(static_cast<BYTE8>('F')); // 9

    EXPECT_EQ(codec.myReadFrameIndex, 0);
    EXPECT_EQ(codec.myWriteFrameIndex, 10);
    codec.put(static_cast<WORD16>(3)); // 10 11
    codec.put(static_cast<BYTE8>('G')); // 12
    codec.put(static_cast<BYTE8>('H')); // 13
    codec.put(static_cast<BYTE8>('I')); // 14

    EXPECT_EQ(codec.myReadFrameIndex, 0);
    EXPECT_EQ(codec.myWriteFrameIndex, 15);
    EXPECT_EQ(codec.getString(), "ABC");
    EXPECT_EQ(codec.myReadFrameIndex, 5);
    EXPECT_EQ(codec.getString(), "DEF");
    EXPECT_EQ(codec.myReadFrameIndex, 10);
    EXPECT_EQ(codec.getString(), "GHI");
    EXPECT_EQ(codec.myReadFrameIndex, 15);
    EXPECT_EQ(codec.myWriteFrameIndex, 15);
}

TEST_F(TestFrameCodec, FrameSizeInvariants) {
    EXPECT_EQ(512, TransactionBufferSize);
    EXPECT_EQ(508, StringBufferSize);
}

TEST_F(TestFrameCodec, GetMaxLengthString) {
    // Note that this ignores that a response frame would have a result byte after the frame size.

    codec.put(static_cast<WORD16>(TransactionBufferSize - 2)); // Max Frame Size, for illustration of a complete frame
    codec.put(static_cast<WORD16>(StringBufferSize)); // Max String Size
    for (int i = 0; i < StringBufferSize; i++) {
        codec.put(static_cast<unsigned char>('A'));
    }

    const WORD16 frameSize = codec.get16(); // For illustration of a complete frame
    EXPECT_EQ(frameSize, 510);

    const std::string str = codec.getString();
    EXPECT_EQ(str.size(), 508);
    for (int i = 0; i < 508; i++) {
        EXPECT_EQ(str[i], 'A');
    }
}

TEST_F(TestFrameCodec, GetStringTooLong) {
    codec.put(static_cast<WORD16>(StringBufferSize + 1)); // Max String Size exceeded
    codec.put(static_cast<BYTE8>('A')); // irrelevant

    EXPECT_THROW(codec.getString(), std::range_error);
}

// FRAME HANDLING

TEST_F(TestFrameCodec, ResetWriteFrame) {
    EXPECT_EQ(codec.myWriteFrameIndex, 0L);
    codec.resetWriteFrame();
    EXPECT_EQ(codec.myWriteFrameIndex, 2L);
}

TEST_F(TestFrameCodec, WriteOffset) {
    BYTE8* initialWriteOffset = codec.writeOffset(0); // whatever it might be
    codec.put(static_cast<BYTE8>(0x00));
    EXPECT_EQ(codec.writeOffset(0), initialWriteOffset); // put hasn't changed it
    EXPECT_EQ(codec.writeOffset(1), initialWriteOffset + 1); // it's all relative to the start of the write buffer
    EXPECT_EQ(codec.writeOffset(2), initialWriteOffset + 2); // it's all relative to the start of the write buffer
}

TEST_F(TestFrameCodec, Advance) {
    for (int i=0; i <= 0x0f; i++) {
        codec.put(static_cast<BYTE8>(i + (i << 4))); // 00 11 22 33 44 55 .. ff
    }
    codec.resetWriteFrame(); // offset is now 2

    codec.advance(6); // offset is now 8
    codec.put(static_cast<BYTE8>(0x00));
    EXPECT_EQ(codec.myTransactionBuffer[7], (BYTE8)0x77);
    EXPECT_EQ(codec.myTransactionBuffer[8], (BYTE8)0x00);
    EXPECT_EQ(codec.myTransactionBuffer[9], (BYTE8)0x99);
}

TEST_F(TestFrameCodec, FillInFrameSize) {
    // Initial size
    EXPECT_EQ(codec.myTransactionBuffer[0], (BYTE8)0x00);
    EXPECT_EQ(codec.myTransactionBuffer[1], (BYTE8)0x00);

    codec.resetWriteFrame();
    codec.put(0xAB03C9AF);
    codec.put(static_cast<WORD16>(0xF00D));

    codec.fillInFrameSize();
    EXPECT_EQ(codec.myTransactionBuffer[0], (BYTE8)0x06);
    EXPECT_EQ(codec.myTransactionBuffer[1], (BYTE8)0x00);
}

TEST_F(TestFrameCodec, FillInReadFrameSize) {
    // Initial size
    EXPECT_EQ(codec.myTransactionBuffer[0], (BYTE8)0x00);
    EXPECT_EQ(codec.myTransactionBuffer[1], (BYTE8)0x00);

    codec.setReadFrameSize(6);
    codec.resetWriteFrame();
    codec.put(static_cast<WORD16>(0xF00D)); // will be overwritten

    codec.fillInReadFrameSize();
    EXPECT_EQ(codec.myTransactionBuffer[0], (BYTE8)0x06);
    EXPECT_EQ(codec.myTransactionBuffer[1], (BYTE8)0x00);
}

TEST_F(TestFrameCodec, WriteFrameSizeInitiallyNotFull) {
    EXPECT_EQ(codec.myWriteFrameIndex, 0L);
    EXPECT_EQ(codec.writeFrameFull(), false);
    codec.resetWriteFrame();
    EXPECT_EQ(codec.writeFrameFull(), false);
}

TEST_F(TestFrameCodec, AlmostFullWriteFrameSizeNotFull) {
    for (int i=0; i < TransactionBufferSize - 1; i++) {
        codec.put(static_cast<BYTE8>(1));
    }
    EXPECT_EQ(codec.writeFrameFull(), false);
}

TEST_F(TestFrameCodec, FullWriteFrameSizeFull) {
    for (int i=0; i < TransactionBufferSize; i++) {
        codec.put(static_cast<BYTE8>(1));
    }
    EXPECT_EQ(codec.writeFrameFull(), true);
}
