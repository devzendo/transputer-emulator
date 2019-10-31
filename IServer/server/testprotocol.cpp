//------------------------------------------------------------------------------
//
// File        : testprotocol.cpp
// Description : Tests the deserialisation of the IServer protocol
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 12/08/2019
//
// (C) 2005-2019 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <stdio.h>
#include "log.h"
#include "platform.h"
#include "protocolhandler.h"
#include "stublink.h"
#include "../isproto.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

class StubPlatform : public Platform {
public:
    StubPlatform();
    void initialise(void) throw (std::exception);
    ~StubPlatform(void);
};
StubPlatform::StubPlatform(): Platform() {}
void StubPlatform::initialise(void) throw(std::exception) {}
StubPlatform::~StubPlatform() = default;

class MockPlatform: public StubPlatform {
    MOCK_METHOD0(isConsoleCharAvailable, bool());
    MOCK_METHOD0(getConsoleChar, BYTE8());
    MOCK_METHOD1(putConsoleChar, void(BYTE8));
    MOCK_METHOD0(getTimeMillis, WORD32());
    MOCK_METHOD0(getUTCTime, UTCTime());
};

class TestProtocolHandler : public ::testing::Test {
protected:
    StubLink stubLink = StubLink(0, false); // false: we are a client of the server we're testing
    MockPlatform mockPlatform;
    ProtocolHandler * handler;

    const std::vector<BYTE8> paddedEmptyFrame = std::vector<BYTE8> {0,0,0,0,0,0};

    void SetUp() override {
        setLogLevel(LOGLEVEL_DEBUG);
        handler = new ProtocolHandler(stubLink, mockPlatform);
        handler->setDebug(true);
    }

    void setReadableIserverMessage(const std::vector<BYTE8> & frame) {
        unsigned long frameSize = frame.size();
        BYTE8 msLength = frameSize / 256;
        BYTE8 lsLength = frameSize - (256 * msLength);
        std::cout << "frame length is " << frameSize << " ms length " << (int)msLength << " ls length " << (int)lsLength << std::endl;
        std::vector<BYTE8> frameVector;
        frameVector.push_back(lsLength);
        frameVector.push_back(msLength);
        frameVector.insert(frameVector.end(), frame.begin(), frame.end());
        stubLink.setReadableBytes(frameVector);
    }

    void checkBadFrame(const std::vector<BYTE8> & badFrame) {
        setReadableIserverMessage(badFrame);
        handler->processFrame();
        EXPECT_EQ(handler->frameCount(), 1L);
        EXPECT_EQ(handler->badFrameCount(), 1L);
    }

    bool checkGoodFrame(const std::vector<BYTE8> & goodFrame) {
        setReadableIserverMessage(goodFrame);
        bool isExit = handler->processFrame();
        EXPECT_EQ(handler->frameCount(), 1L);
        EXPECT_EQ(handler->badFrameCount(), 0L);
        return isExit;
    }

    std::vector<BYTE8> & padFrame(std::vector<BYTE8> & unpaddedFrame) {
        while (unpaddedFrame.size() < 6 || ((unpaddedFrame.size() & 0x01) == 0x01))  {
            unpaddedFrame.push_back(0x00);
        }
        return unpaddedFrame;
    }

    std::vector<BYTE8> & padTo(int toSize, std::vector<BYTE8> & frame) {
        while (frame.size() != toSize)  {
            frame.push_back(0x00);
        }
        return frame;
    }
};

TEST_F(TestProtocolHandler, InitialFrameCounts)
{
    EXPECT_EQ(handler->frameCount(), 0L);
    EXPECT_EQ(handler->badFrameCount(), 0L);
}

TEST_F(TestProtocolHandler, ShortFrame0IsShort)
{
    checkBadFrame((std::vector<BYTE8>) {});
}

TEST_F(TestProtocolHandler, ShortFrame1IsShort)
{
    checkBadFrame((std::vector<BYTE8>) {0});
}

TEST_F(TestProtocolHandler, ShortFrame4IsShort)
{
    checkBadFrame((std::vector<BYTE8>) {0, 0, 0, 0});
}

TEST_F(TestProtocolHandler, ShortFrame5IsOdd)
{
    checkBadFrame((std::vector<BYTE8>) {0, 0, 0, 0, 0});
}

TEST_F(TestProtocolHandler, OddFrame7IsOdd)
{
    checkBadFrame((std::vector<BYTE8>) {0, 0, 0, 0, 0, 0, 0});
}

TEST_F(TestProtocolHandler, MaxFrame510IsGood)
{
    std::vector<BYTE8> maxFrame;
    padTo(510, maxFrame);

    checkGoodFrame(maxFrame);
}

TEST_F(TestProtocolHandler, LongFrame511IsOddlyBad)
{
    std::vector<BYTE8> maxFrame;
    padTo(511, maxFrame);

    checkBadFrame(maxFrame);
}

TEST_F(TestProtocolHandler, LongFrame512IsBad)
{
    std::vector<BYTE8> maxFrame;
    padTo(512, maxFrame);

    checkBadFrame(maxFrame);
}

TEST_F(TestProtocolHandler, PadFrame0To6)
{
    std::vector<BYTE8> emptyFrame;

    EXPECT_EQ(padFrame(emptyFrame), paddedEmptyFrame);
}

TEST_F(TestProtocolHandler, PadFrame1To6)
{
    std::vector<BYTE8> shortFrame = {0};

    EXPECT_EQ(padFrame(shortFrame), paddedEmptyFrame);
}

TEST_F(TestProtocolHandler, PadFrame2To6)
{
    std::vector<BYTE8> shortFrame = {0, 0};

    EXPECT_EQ(padFrame(shortFrame), paddedEmptyFrame);
}

TEST_F(TestProtocolHandler, PadFrame3To6)
{
    std::vector<BYTE8> shortFrame = {0, 0, 0};

    EXPECT_EQ(padFrame(shortFrame), paddedEmptyFrame);
}

TEST_F(TestProtocolHandler, PadFrame4To6)
{
    std::vector<BYTE8> shortFrame = {0, 0, 0, 0};

    EXPECT_EQ(padFrame(shortFrame), paddedEmptyFrame);
}

TEST_F(TestProtocolHandler, PadFrame5To6)
{
    std::vector<BYTE8> shortFrame = {0, 0, 0, 0, 0};

    EXPECT_EQ(padFrame(shortFrame), paddedEmptyFrame);
}

TEST_F(TestProtocolHandler, PadFrame6DoesNothing)
{
    std::vector<BYTE8> okFrame = {0, 0, 0, 0, 0, 0};

    EXPECT_EQ(padFrame(okFrame), paddedEmptyFrame);
}

TEST_F(TestProtocolHandler, PadFrame7ToEvenSize)
{
    std::vector<BYTE8> sevenBytes = {0, 0, 0, 0, 0, 0, 0};
    const std::vector<BYTE8> eightBytes = {0, 0, 0, 0, 0, 0, 0, 0};
    EXPECT_EQ(padFrame(sevenBytes), eightBytes);
}

TEST_F(TestProtocolHandler, IdFrame)
{
    std::vector<BYTE8> idFrame = {REQ_ID};
    std::vector<BYTE8> padded = padFrame(idFrame);
    EXPECT_EQ(checkGoodFrame(padded), false); // Not an exit frame
}

TEST_F(TestProtocolHandler, ExitFrame)
{
    std::vector<BYTE8> exitFrame = {REQ_EXIT};
    std::vector<BYTE8> padded = padFrame(exitFrame);
    EXPECT_EQ(checkGoodFrame(padded), true); // It is an exit frame
}

// TODO a good 6 byte frame
// TODO a good 510 byte frame
// TODO a bad 511 byte frame

// TODO rounding up an odd-length in-range frame
