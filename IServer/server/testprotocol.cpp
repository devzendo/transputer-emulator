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

#include <cstdio>
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
        bool isExit = sendFrame(goodFrame);
        EXPECT_EQ(handler->frameCount(), 1L);
        EXPECT_EQ(handler->badFrameCount(), 0L);
        return isExit;
    }

    bool sendFrame(const std::vector<BYTE8> & frame) {
        setReadableIserverMessage(frame);
        bool isExit = handler->processFrame();
        return isExit;
    }

    void checkResponseFrameTag(const std::vector<BYTE8> &frame, const BYTE8 expectedResponseFrameTag) {
        int actualResponseFrameTag = (int) frame[2];
        EXPECT_EQ(actualResponseFrameTag, (int)expectedResponseFrameTag);
    }

    void checkResponseFrameSize(const std::vector<BYTE8> & frame, const WORD16 expectedResponseFrameSize) {
        WORD16 actualResponseFrameSize = frame[0] + (frame[1]*256);
        EXPECT_EQ(actualResponseFrameSize, expectedResponseFrameSize);
    }

    std::vector<BYTE8> readResponseFrame() {
        // The protocol request/response is synchronous. So if the request was handled, there'll be
        // a response.
        return stubLink.getWrittenBytes();
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

    std::vector<BYTE8> & append8(std::vector<BYTE8> &frame, const BYTE8 b) {
        frame.push_back(b);
        return frame;
    };

    std::vector<BYTE8> & append16(std::vector<BYTE8> &frame, const WORD16 w) {
        // Always output as a little-endian word, LSB first MSB last
        frame.push_back(w & 0x00ff);
        frame.push_back((w & 0xff00) >> 8);
        return frame;
    };

    std::vector<BYTE8> & append32(std::vector<BYTE8> &frame, const WORD32 w) {
        // Always output as a little-endian word, LSB first MSB last
        frame.push_back(w & 0x000000ff);
        frame.push_back((w & 0x0000ff00) >> 8);
        frame.push_back((w & 0x00ff0000) >> 16);
        frame.push_back((w & 0xff000000) >> 24);
        return frame;
    };

    std::vector<BYTE8> & appendString(std::vector<BYTE8> &frame, const std::string &str) {
        append16(frame, str.length());
        auto it=str.begin();
        auto end=str.end();
        for (; it != end; it++) {
            frame.push_back(*it);
        }
        return frame;
    };
};

// FRAME HANDLING

TEST_F(TestProtocolHandler, InitialFrameCounts)
{
    EXPECT_EQ(handler->frameCount(), 0L);
    EXPECT_EQ(handler->badFrameCount(), 0L);
}

TEST_F(TestProtocolHandler, ShortFrame0IsShort)
{
    auto badFrame = std::vector<BYTE8> {};
    checkBadFrame(badFrame);
}

TEST_F(TestProtocolHandler, ShortFrame1IsShort)
{
    auto badFrame = std::vector<BYTE8> {0};
    checkBadFrame(badFrame);
}

TEST_F(TestProtocolHandler, ShortFrame4IsShort)
{
    auto badFrame = std::vector<BYTE8> {0, 0, 0, 0};
    checkBadFrame(badFrame);
}

TEST_F(TestProtocolHandler, ShortFrame5IsOdd)
{
    auto badFrame = std::vector<BYTE8> {0, 0, 0, 0, 0};
    checkBadFrame(badFrame);
}

TEST_F(TestProtocolHandler, OddFrame7IsOdd)
{
    auto badFrame = std::vector<BYTE8> {0, 0, 0, 0, 0, 0, 0};
    checkBadFrame(badFrame);
}

// TODO this test may cause debug assertion dialogs on Windows.. iscprint?!
TEST_F(TestProtocolHandler, MaxFrame510IsGood)
{
    std::vector<BYTE8> maxFrame;
    padTo(510, maxFrame);

    checkGoodFrame(maxFrame);
    std::vector<BYTE8> response = readResponseFrame();
    checkResponseFrameTag(response, RES_UNIMPLEMENTED);
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

TEST_F(TestProtocolHandler, OddSizeResponseFrameIsPaddedWithZero)
{
    std::vector<BYTE8> idFrame = {REQ_ID};
    sendFrame(padFrame(idFrame));
    std::vector<BYTE8> response = readResponseFrame();
    checkResponseFrameSize(response, 6);
    checkResponseFrameTag(response, RES_SUCCESS);
    EXPECT_EQ((int)response[7], 0x00); // padding
}

// UNIMPLEMENTED

TEST_F(TestProtocolHandler, UnimplementedFrame)
{
    std::vector<BYTE8> unimplementedFrame = {9}; // an unimplemented tag
    std::vector<BYTE8> padded = padFrame(unimplementedFrame);
    EXPECT_EQ(checkGoodFrame(padded), false); // its length is good, it's not an exit frame
    EXPECT_EQ(handler->unimplementedFrameCount(), 1L); // but it isn't an implemented tag
    std::vector<BYTE8> response = readResponseFrame();
    checkResponseFrameSize(response, 1 + 1); // padded
    checkResponseFrameTag(response, RES_UNIMPLEMENTED);
}

// REQ_OPEN

// TODO come back to this... after puts/gets to stdout/stderr/stdin
//TEST_F(TestProtocolHandler, OpenOpensAFileAndReturnsAStream)
//{
//    std::vector<BYTE8> openFrame = {REQ_OPEN};
//    appendString(openFrame, "testfile.txt");
//    append8(openFrame, REQ_OPEN_TYPE_TEXT);
//    append8(openFrame, REQ_OPEN_MODE_INPUT);
//    std::vector<BYTE8> padded = padFrame(openFrame);
//    EXPECT_FALSE(checkGoodFrame(padded)); // It is an exit frame
//    EXPECT_EQ(handler->unimplementedFrameCount(), 0L); // it is an implemented tag
//
//    std::vector<BYTE8> response = readResponseFrame();
//    checkResponseFrameTag(response, RES_SUCCESS);
//    checkResponseFrameSize(response, 4);
//    EXPECT_EQ((int)response[3], 0x03); // First available stream id after 0,1,2 (stdout, stdin, stderr)
//    EXPECT_EQ((int)response[4], 0x00);
//    EXPECT_EQ((int)response[5], 0x00);
//    EXPECT_EQ((int)response[6], 0x00);
//}

// REQ_CLOSE
// REQ_READ

// REQ_WRITE
// REQ_GETS
// REQ_PUTS
// REQ_FLUSH
// REQ_SEEK
// REQ_TELL
// REQ_EOF
// REQ_FERROR
// REQ_REMOVE
// REQ_RENAME
// REQ_GETBLOCK
// REQ_PUTBLOCK
// REQ_ISATTY
// REQ_OPENREC
// REQ_GETREC
// REQ_PUTREC
// REQ_PUTEOF
// REQ_GETKEY
// REQ_POLLKEY

// REQ_GETENV
// REQ_TIME
// REQ_SYSTEM

// REQ_EXIT

TEST_F(TestProtocolHandler, ExitFrameSuccess)
{
    std::vector<BYTE8> exitFrame = {REQ_EXIT};
    append32(exitFrame, RES_EXIT_SUCCESS);
    std::vector<BYTE8> padded = padFrame(exitFrame);
    EXPECT_EQ(checkGoodFrame(padded), true); // It is an exit frame
    std::vector<BYTE8> response = readResponseFrame();
    checkResponseFrameTag(response, RES_SUCCESS);
    checkResponseFrameSize(response, 2);
    EXPECT_EQ(handler->unimplementedFrameCount(), 0L); // it is an implemented tag

    EXPECT_EQ(handler->exitCode(), 0);
}

TEST_F(TestProtocolHandler, ExitFrameFailure)
{
    std::vector<BYTE8> exitFrame = {REQ_EXIT};
    append32(exitFrame, RES_EXIT_FAILURE);
    std::vector<BYTE8> padded = padFrame(exitFrame);
    EXPECT_EQ(checkGoodFrame(padded), true); // It is an exit frame
    std::vector<BYTE8> response = readResponseFrame();
    checkResponseFrameTag(response, RES_SUCCESS);
    checkResponseFrameSize(response, 2);
    EXPECT_EQ(handler->unimplementedFrameCount(), 0L); // it is an implemented tag

    EXPECT_EQ(handler->exitCode(), 1);
}

TEST_F(TestProtocolHandler, ExitFrameCustom)
{
    std::vector<BYTE8> exitFrame = {REQ_EXIT};
    append32(exitFrame, 0x12345678);
    std::vector<BYTE8> padded = padFrame(exitFrame);
    EXPECT_EQ(checkGoodFrame(padded), true); // It is an exit frame
    std::vector<BYTE8> response = readResponseFrame();
    checkResponseFrameTag(response, RES_SUCCESS);
    checkResponseFrameSize(response, 2);
    EXPECT_EQ(handler->unimplementedFrameCount(), 0L); // it is an implemented tag

    EXPECT_EQ(handler->exitCode(), 0x12345678);
}

// REQ_COMMAND
// REQ_CORE
// REQ_ID



TEST_F(TestProtocolHandler, IdFrame)
{
    std::vector<BYTE8> idFrame = {REQ_ID};
    std::vector<BYTE8> padded = padFrame(idFrame);
    EXPECT_EQ(checkGoodFrame(padded), false); // its length is good, it's not an exit frame
    EXPECT_EQ(handler->unimplementedFrameCount(), 0L); // it is an implemented tag
    std::vector<BYTE8> response = readResponseFrame();
    checkResponseFrameSize(response, 6);
    checkResponseFrameTag(response, RES_SUCCESS);
    EXPECT_EQ((int)response[3], 0x00); // version TODO extract real version from productVersion
#if defined(PLATFORM_OSX)
    EXPECT_EQ((int)response[4], 0x09); // host
    EXPECT_EQ((int)response[5], 0x07); // os
#elif defined(PLATFORM_LINUX)
    EXPECT_EQ((int)response[4], 0x01); // host
    EXPECT_EQ((int)response[5], 0x08); // os
#elif defined(PLATFORM_WINDOWS)
    EXPECT_EQ((int)response[4], 0x01); // host
    EXPECT_EQ((int)response[5], 0x06); // os
#else
    FAIL() << "Platform not supported";
#endif
    EXPECT_EQ((int)response[6], LinkType_Stub); // board a.k.a. link type
}

// REQ_GETINFO

// REQ_MSDOS

// REQ_FILEEXISTS
// REQ_TRANSLATE
// REQ_FERRSTAT
// REQ_COMMANDARG

// TODO a good 510 byte frame
// TODO a bad 511 byte frame
