//------------------------------------------------------------------------------
//
// File        : testprotocolhandler.cpp
// Description : Tests the deserialisation of the IServer protocol
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 12/08/2019
//
// (C) 2005-2024 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <fstream>

#include <hexdump.h>
#include "log.h"
#include "platform.h"
#include "protocolhandler.h"
#include "stublink.h"
#include "filesystem.h"
#include "../isproto.h"

#include "gtest/gtest.h"
#include "memstreambuf.h"
#include "tempfilesfixture.h"

class StubPlatform final : public Platform {
public:
    StubPlatform();
    void initialise() noexcept(false) override;
    ~StubPlatform() override;

    // From keyboard..
    bool isConsoleCharAvailable() override;
    BYTE8 getConsoleChar() override;
    void putKeyboardChar(BYTE8 ch); // for tests

    // To screen..
    void putConsoleChar(BYTE8 ch) override;
    std::vector<BYTE8> getScreenChars(); // for tests

    WORD32 getTimeMillis() override;
    void setTimeMillis(WORD32 timeMillis); // for tests
    UTCTime getUTCTime() override;
    void setUTCTime(UTCTime utcTime); // for tests

    std::vector<BYTE8> getCommandLineAll();
    std::vector<BYTE8> getCommandLineForProgram();
    void setCommandLine(std::string text); // for tests
private:
    std::vector<BYTE8> myKeyboardChars;
    std::vector<BYTE8> myScreenChars;
    WORD32 myTimeMillis;
    UTCTime myUTCTime;
};
StubPlatform::StubPlatform(): Platform() {
    myTimeMillis = 0L;
}
void StubPlatform::initialise() noexcept(false) {}
StubPlatform::~StubPlatform() = default;
// From keyboard..
bool StubPlatform::isConsoleCharAvailable() {
    return !myKeyboardChars.empty();
}
BYTE8 StubPlatform::getConsoleChar() {
    return myKeyboardChars.front();
}
void StubPlatform::putKeyboardChar(const BYTE8 ch) {
    myKeyboardChars.push_back(ch);
}

// To screen..
void StubPlatform::putConsoleChar(const BYTE8 ch) {
    myScreenChars.push_back(ch);
}
std::vector<BYTE8> StubPlatform::getScreenChars() {
    return myScreenChars;
}

WORD32 StubPlatform::getTimeMillis() {
    return myTimeMillis;
}
void StubPlatform::setTimeMillis(const WORD32 timeMillis) {
    myTimeMillis = timeMillis;
}
UTCTime StubPlatform::getUTCTime() {
    return myUTCTime;
}
void StubPlatform::setUTCTime(const UTCTime utcTime) {
    myUTCTime = utcTime;
}


class TestProtocolHandler : public TestTempFiles, public ::testing::Test {
protected:
    StubLink stubLink = StubLink(0, false); // false: we are a client of the server we're testing
    StubPlatform stubPlatform;
    ProtocolHandler * handler;
    std::string tempDir;

    const std::vector<BYTE8> paddedEmptyFrame = std::vector<BYTE8> {0,0,0,0,0,0};

    void SetUp() override {
        setLogLevel(LOGLEVEL_DEBUG);
        tempDir = tempdir();
        handler = new ProtocolHandler(stubLink, stubPlatform, tempDir);
        handler->setDebug(true);
    }

    void TearDown() override {
        removeTempFiles();
    }

    void setReadableIserverMessage(const std::vector<BYTE8> & frame) {
        unsigned long frameSize = frame.size();
        BYTE8 msLength = frameSize / 256;
        BYTE8 lsLength = frameSize - (256 * msLength);
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

    // Precondition: goodFrame is a valid frame
    // Sends a frame, checks that it's valid and has been processed not rejected.
    // Return: true iff the frame sent is an exit frame.
    bool checkGoodFrame(const std::vector<BYTE8> & goodFrame) {
        bool isExit = sendFrame(goodFrame);
        EXPECT_EQ(handler->frameCount(), 1L);
        EXPECT_EQ(handler->badFrameCount(), 0L);
        return isExit;
    }

    // Return: true iff the frame sent is an exit frame.
    bool sendFrame(const std::vector<BYTE8> & frame) {
        setReadableIserverMessage(frame);
        bool isExit = handler->processFrame();
        return isExit;
    }

    // Return: true iff the frame sent is an exit frame.
    bool padAndSendFrame(std::vector<BYTE8> & unpaddedFrame) {
        return sendFrame(padFrame(unpaddedFrame));
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

    BYTE8 get8(const std::vector<BYTE8> & frame, const int offset) {
        return frame[offset];
    }

    WORD16 get16(const std::vector<BYTE8> & frame, const int offset) {
        // Always output as a little-endian word, LSB first MSB last
        return frame[offset] +
              (frame[offset+1] << 8);
    }

    WORD16 get32(const std::vector<BYTE8> & frame, const int offset) {
        // Always output as a little-endian word, LSB first MSB last
        return frame[offset] +
              (frame[offset+1] << 8) +
              (frame[offset+2] << 16) +
              (frame[offset+3] << 24);
    }

    // Get the stream Id from a successful open
    WORD32 getStreamId() {
        std::vector<BYTE8> openResponse = this->readResponseFrame();
        this->checkResponseFrameTag(openResponse, RES_SUCCESS);
        this->checkResponseFrameSize(openResponse, 6);
        WORD32 streamId = this->get32(openResponse, 3);
        return streamId;
    }

    // Common test code for testing \n translation in 'open text for output' tests
    std::string openTextOutputTranslation(const std::string& writtenString, const unsigned short expectedWrittenBytes) {
        std::string testFileName = createRandomTempFileName();
        std::string testFilePath = pathJoin(tempdir(), testFileName);

        std::vector<BYTE8> openFrame = {REQ_OPEN};
        appendString(openFrame, testFileName);
        append8(openFrame, REQ_OPEN_TYPE_TEXT);
        append8(openFrame, REQ_OPEN_MODE_OUTPUT);
        sendFrame(padFrame(openFrame));

        std::vector<BYTE8> openResponse = readResponseFrame();
        checkResponseFrameTag(openResponse, RES_SUCCESS);
        checkResponseFrameSize(openResponse, 6);
        WORD32 streamId = get32(openResponse, 3);
        EXPECT_EQ(streamId, 3); // First available stream id after 0,1,2 (stdout, stdin, stderr)

        // write A\nB to streamId 3
        std::vector<BYTE8> writeFrame = {REQ_WRITE};
        append32(writeFrame, streamId);
        appendString(writeFrame, writtenString);
        sendFrame(padFrame(writeFrame));

        std::vector<BYTE8> writeResponse = readResponseFrame();
        checkResponseFrameTag(writeResponse, RES_SUCCESS);
        checkResponseFrameSize(writeResponse, 4); // RES_SUCCESS + 0 + 0 + 0-pad
        WORD16 written = get16(writeResponse, 3);
        EXPECT_EQ(written, expectedWrittenBytes);

        // close streamId 3
        std::vector<BYTE8> closeFrame = {REQ_CLOSE};
        append32(closeFrame, streamId);
        sendFrame(padFrame(closeFrame));

        std::vector<BYTE8> closeResponse = readResponseFrame();
        checkResponseFrameTag(closeResponse, RES_SUCCESS);
        checkResponseFrameSize(closeResponse, 2); // RES_SUCCESS + 0-pad

        return testFilePath;
    }
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

TEST_F(TestProtocolHandler, OpenInputOpensAFileAndReturnsAStream)
{
    std::string testFileName = createRandomTempFileName();
    std::string testFilePath = pathJoin(tempdir(), testFileName);
    createTempFile(testFilePath, "ABCD");

    std::vector<BYTE8> openFrame = {REQ_OPEN};
    appendString(openFrame, testFileName);
    append8(openFrame, REQ_OPEN_TYPE_TEXT);
    append8(openFrame, REQ_OPEN_MODE_INPUT);
    std::vector<BYTE8> padded = padFrame(openFrame);
    bool wasSensedAsExitFrame = checkGoodFrame(padded);
    EXPECT_FALSE(wasSensedAsExitFrame);
    EXPECT_EQ(handler->unimplementedFrameCount(), 0L); // it is an implemented tag

    std::vector<BYTE8> response = readResponseFrame();
    checkResponseFrameTag(response, RES_SUCCESS);
    checkResponseFrameSize(response, 6);
    EXPECT_EQ((int)response[3], 0x03); // First available stream id after 0,1,2 (stdout, stdin, stderr)
    EXPECT_EQ((int)response[4], 0x00);
    EXPECT_EQ((int)response[5], 0x00);
    EXPECT_EQ((int)response[6], 0x00);
}

TEST_F(TestProtocolHandler, OpenInputFailsWhenFileDoesNotExist)
{
    std::string nonExistantTestFilePath = pathJoin(tempdir(), createRandomTempFileName());

    std::vector<BYTE8> openFrame = {REQ_OPEN};
    appendString(openFrame, nonExistantTestFilePath);
    append8(openFrame, REQ_OPEN_TYPE_TEXT);
    append8(openFrame, REQ_OPEN_MODE_INPUT);
    padAndSendFrame(openFrame);

    std::vector<BYTE8> response = readResponseFrame();
    checkResponseFrameTag(response, RES_ERROR);
    checkResponseFrameSize(response, 4);
    EXPECT_EQ((int)response[3], 0x00);
    EXPECT_EQ((int)response[4], 0x00);
}

TEST_F(TestProtocolHandler, OpenOutputOpensAFileAndReturnsAStreamThatCanBeWrittenTo)
{
    std::string testFileName = createRandomTempFileName();
    std::string testFilePath = pathJoin(tempdir(), testFileName);

    // Open
    std::vector<BYTE8> openFrame = {REQ_OPEN};
    appendString(openFrame, testFileName);
    append8(openFrame, REQ_OPEN_TYPE_BINARY);
    append8(openFrame, REQ_OPEN_MODE_OUTPUT);
    std::vector<BYTE8> padded = padFrame(openFrame);
    bool wasSensedAsExitFrame = checkGoodFrame(padded);
    EXPECT_FALSE(wasSensedAsExitFrame);
    EXPECT_EQ(handler->unimplementedFrameCount(), 0L); // it is an implemented tag

    std::vector<BYTE8> openResponse = readResponseFrame();
    checkResponseFrameTag(openResponse, RES_SUCCESS);
    checkResponseFrameSize(openResponse, 6);
    WORD32 streamId = get32(openResponse, 3);
    EXPECT_EQ(streamId, 3); // First available stream id after 0,1,2 (stdout, stdin, stderr)

    // Write
    std::vector<BYTE8> writeFrame = {REQ_WRITE};
    append32(writeFrame, streamId);
    appendString(writeFrame, "ABCD");
    sendFrame(padFrame(writeFrame));

    std::vector<BYTE8> writeResponse = readResponseFrame();
    checkResponseFrameTag(writeResponse, RES_SUCCESS);
    checkResponseFrameSize(writeResponse, 4); // RES_SUCCESS + 0 + 0 + 0-pad
    WORD16 written = get16(writeResponse, 3);
    EXPECT_EQ(written, 4);

    // Close streamId 3
    std::vector<BYTE8> closeFrame = {REQ_CLOSE};
    append32(closeFrame, streamId);
    sendFrame(padFrame(closeFrame));

    std::vector<BYTE8> closeResponse = readResponseFrame();
    checkResponseFrameTag(closeResponse, RES_SUCCESS);
    checkResponseFrameSize(closeResponse, 2); // RES_SUCCESS + 0-pad

    // read file and check it contains ABCD
    EXPECT_EQ(readFileContents(testFilePath), "ABCD");
}

TEST_F(TestProtocolHandler, OpenOutputCannotBeReadFrom)
{
    const std::pair<std::string, std::string> &testFilePathAndName = createRandomTempFilePathContaining();
    const std::string &testFileName = testFilePathAndName.second;

    // Open
    std::vector<BYTE8> openFrame = {REQ_OPEN};
    appendString(openFrame, testFileName);
    append8(openFrame, REQ_OPEN_TYPE_BINARY);
    append8(openFrame, REQ_OPEN_MODE_OUTPUT);
    sendFrame(padFrame(openFrame));

    std::vector<BYTE8> openResponse = readResponseFrame();
    checkResponseFrameTag(openResponse, RES_SUCCESS);
    WORD32 streamId = get32(openResponse, 3);

    // Read will fail
    std::vector<BYTE8> readFrame = {REQ_READ};
    append32(readFrame, streamId);
    append16(readFrame, 4);
    sendFrame(padFrame(readFrame));

    std::vector<BYTE8> readResponse = readResponseFrame();
    checkResponseFrameTag(readResponse, RES_BADID);
    checkResponseFrameSize(readResponse, 4); // RES_BADID + 0 + 0 + 0-pad
    WORD16 read = get16(readResponse, 3);
    EXPECT_EQ(read, 0);
}

TEST_F(TestProtocolHandler, OpenInputCannotBeWrittenTo)
{
    const std::pair<std::string, std::string> &testFilePathAndName = createRandomTempFilePathContaining("ABCD");
    const std::string &testFileName = testFilePathAndName.second;

    // Open
    std::vector<BYTE8> openFrame = {REQ_OPEN};
    appendString(openFrame, testFileName);
    append8(openFrame, REQ_OPEN_TYPE_BINARY);
    append8(openFrame, REQ_OPEN_MODE_INPUT);
    sendFrame(padFrame(openFrame));

    std::vector<BYTE8> openResponse = readResponseFrame();
    checkResponseFrameTag(openResponse, RES_SUCCESS);
    WORD32 streamId = get32(openResponse, 3);

    // Write will fail
    std::vector<BYTE8> writeFrame = {REQ_WRITE};
    append32(writeFrame, streamId);
    append16(writeFrame, 4);
    sendFrame(padFrame(writeFrame));

    std::vector<BYTE8> writeResponse = readResponseFrame();
    checkResponseFrameTag(writeResponse, RES_BADID);
    checkResponseFrameSize(writeResponse, 4); // RES_BADID + 0 + 0 + 0-pad
    WORD16 written = get16(writeResponse, 3);
    EXPECT_EQ(written, 0);
}

// Text files only make a distinction on Windows; text and binary processing is identical on OSX/Linux.
// See tests in testcharacterisation.cpp for explanation.

#if defined(PLATFORM_WINDOWS)
TEST_F(TestProtocolHandler, OpenTextTranslatesLineFeedToCarriageReturnLineFeedOnWindows)
{
    std::string writtenString = "A\nB";

    // A\nB is expanded to A\r\nB in the file, but from the iostream API's perspective,
    // we only write 3 bytes.
    WORD16 expectedWrittenBytes = 3;

    std::string testFilePath = openTextOutputTranslation(writtenString, expectedWrittenBytes);

    const std::string &contents = readFileContents(testFilePath);
    logDebugF("The read data is (%d bytes):", contents.size());
    hexdump((unsigned char *) contents.c_str(), contents.size());
    // Text mode should expand the written \n to \r\n on Windows
    std::string readString = "A\r\nB";
    EXPECT_EQ(contents, readString);
}
#endif

#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
TEST_F(TestProtocolHandler, OpenTextDoesNotTranslateLineFeedToCarriageReturnLineFeedOnNonWindows)
{
    std::string writtenString = "A\nB";

    // A\nB is kept as is
    WORD16 expectedWrittenBytes = 3;
    // Text mode should not expand the written \n to \r\n on Non-Windows
    std::string readString = "A\nB";

    std::string testFilePath = openTextOutputTranslation(writtenString, expectedWrittenBytes);

    const std::string &contents = readFileContents(testFilePath);
    logDebug("The read data is:");
    hexdump((unsigned char *) contents.c_str(), contents.size());
    EXPECT_EQ(contents, readString);
}
// TODO write a file with A\r\nB on Windows, open text and read, should get A\nB ? Add characterisation.
// TODO same for non-Windows
#endif

// TODO binary files
// TODO REQ_OPEN_MODE_INPUT:
// TODO REQ_OPEN_MODE_OUTPUT:
// TODO REQ_OPEN_MODE_APPEND:
// TODO REQ_OPEN_MODE_EXISTING_UPDATE:
// TODO REQ_OPEN_MODE_NEW_UPDATE:
// TODO REQ_OPEN_MODE_APPEND_UPDATE:

// REQ_CLOSE
TEST_F(TestProtocolHandler, CloseOpenFileClosesWithSuccessThenFailsIfAlreadyClosed) {
    std::string testFileName = createRandomTempFileName();
    std::string testFilePath = pathJoin(tempdir(), testFileName);

    std::vector<BYTE8> openFrame = {REQ_OPEN};
    appendString(openFrame, testFileName);
    append8(openFrame, REQ_OPEN_TYPE_TEXT);
    append8(openFrame, REQ_OPEN_MODE_OUTPUT);
    sendFrame(padFrame(openFrame));

    std::vector<BYTE8> openResponse = readResponseFrame();
    checkResponseFrameTag(openResponse, RES_SUCCESS);
    checkResponseFrameSize(openResponse, 6);
    WORD32 streamId = get32(openResponse, 3);
    EXPECT_EQ(streamId, 3); // First available stream id after 0,1,2 (stdout, stdin, stderr)

    // close streamId 3
    std::vector<BYTE8> initialCloseFrame = {REQ_CLOSE};
    append32(initialCloseFrame, streamId);
    sendFrame(padFrame(initialCloseFrame));

    std::vector<BYTE8> initialCloseResponse = readResponseFrame();
    checkResponseFrameTag(initialCloseResponse, RES_SUCCESS);
    checkResponseFrameSize(initialCloseResponse, 2); // RES_SUCCESS + 0-pad

    // close streamId 3 again, it should fail
    std::vector<BYTE8> secondCloseFrame = {REQ_CLOSE};
    append32(secondCloseFrame, streamId);
    sendFrame(padFrame(secondCloseFrame));

    std::vector<BYTE8> secondCloseResponse = readResponseFrame();
    checkResponseFrameTag(secondCloseResponse, RES_BADID);
    checkResponseFrameSize(secondCloseResponse, 2); // RES_BADID + 0-pad
}

TEST_F(TestProtocolHandler, CloseFailsIfNeverOpened) {
    // streamId 3 has not been opened, it should fail
    WORD32 streamId = 3;
    std::vector<BYTE8> closeFrame = {REQ_CLOSE};
    append32(closeFrame, streamId);
    sendFrame(padFrame(closeFrame));

    std::vector<BYTE8> closeResponse = readResponseFrame();
    checkResponseFrameTag(closeResponse, RES_BADID);
    checkResponseFrameSize(closeResponse, 2); // RES_BADID + 0-pad
}

TEST_F(TestProtocolHandler, CloseFailsIfUnderlyingCloseFails)
{
    std::string testFileName = createRandomTempFileName();
    std::string testFilePath = pathJoin(tempdir(), testFileName);

    std::vector<BYTE8> openFrame = {REQ_OPEN};
    appendString(openFrame, testFileName);
    append8(openFrame, REQ_OPEN_TYPE_TEXT);
    append8(openFrame, REQ_OPEN_MODE_OUTPUT);
    sendFrame(padFrame(openFrame));

    WORD32 streamId = this->getStreamId();
    EXPECT_EQ(streamId, 3); // First available stream id after 0,1,2 (stdout, stdin, stderr)

    // How to get fstream close to fail for a real failure-to-close?
    // Set a streams basic_filebuf to a freshly constructed one, then its __file_ will be 0 so
    // close() returns 0, so fstream close() sets failbit
    std::basic_filebuf<char> emptiness;
    stubPlatform._setFileBuf((int) streamId, emptiness);

    std::vector<BYTE8> closeFrame = {REQ_CLOSE};
    append32(closeFrame, streamId);
    sendFrame(padFrame(closeFrame));

    std::vector<BYTE8> closeResponse = readResponseFrame();
    checkResponseFrameTag(closeResponse, RES_ERROR);
    checkResponseFrameSize(closeResponse, 2); // RES_ERROR + 0-pad
}

TEST_F(TestProtocolHandler, CloseNullifiesStreamsSoTheyCanBeReallocated) {
    std::string testFileName = createRandomTempFileName();
    std::string testFilePath = pathJoin(tempdir(), testFileName);

    std::vector<BYTE8> firstOpenFrame = {REQ_OPEN};
    appendString(firstOpenFrame, testFileName);
    append8(firstOpenFrame, REQ_OPEN_TYPE_TEXT);
    append8(firstOpenFrame, REQ_OPEN_MODE_OUTPUT);
    sendFrame(padFrame(firstOpenFrame));

    std::vector<BYTE8> firstOpenResponse = readResponseFrame();
    WORD32 firstStreamId = get32(firstOpenResponse, 3);
    EXPECT_EQ(firstStreamId, 3); // First available stream id after 0,1,2 (stdout, stdin, stderr)

    // close streamId 3
    std::vector<BYTE8> closeFrame = {REQ_CLOSE};
    append32(closeFrame, firstStreamId);
    sendFrame(padFrame(closeFrame));

    std::vector<BYTE8> closeResponse = readResponseFrame();
    checkResponseFrameTag(closeResponse, RES_SUCCESS);

    // Reopen file, should get streamId 3 again
    std::vector<BYTE8> secondOpenFrame = {REQ_OPEN};
    appendString(secondOpenFrame, testFileName);
    append8(secondOpenFrame, REQ_OPEN_TYPE_TEXT);
    append8(secondOpenFrame, REQ_OPEN_MODE_OUTPUT);
    sendFrame(padFrame(secondOpenFrame));

    std::vector<BYTE8> secondOpenResponse = readResponseFrame();
    WORD32 secondStreamId = get32(secondOpenResponse, 3);
    EXPECT_EQ(secondStreamId, firstStreamId); // Same as before
}

// Note: You could send a REQ_CLOSE to close stdin/out/err but ConsoleStream's close() is a no-op currently.

// REQ_READ
TEST_F(TestProtocolHandler, ReadHandling)
{
    std::vector<BYTE8> readFrame = {REQ_READ};
    append32(readFrame, 3); // This stream isn't open.
    appendString(readFrame, "XXXX");
    std::vector<BYTE8> padded = padFrame(readFrame);
    bool wasSensedAsExitFrame = checkGoodFrame(padded);
    EXPECT_FALSE(wasSensedAsExitFrame);
    EXPECT_EQ(handler->unimplementedFrameCount(), 0L); // it is an implemented tag
}

TEST_F(TestProtocolHandler, ReadFromUnopenFileIsUnsuccessful)
{
    std::vector<BYTE8> readFrame = {REQ_READ};
    append32(readFrame, 3); // This stream isn't open.
    appendString(readFrame, "XXXX");
    std::vector<BYTE8> padded = padFrame(readFrame);
    sendFrame(padded);

    std::vector<BYTE8> response = readResponseFrame();
    checkResponseFrameTag(response, RES_BADID);
    checkResponseFrameSize(response, 4); // RES_BADID + 0 + 0 + 0-pad
    EXPECT_EQ((int)response[3], 0x00);
    EXPECT_EQ((int)response[4], 0x00);
}

TEST_F(TestProtocolHandler, ReadFromNegativeOutOfRangeFileIsUnsuccessful)
{
    std::vector<BYTE8> readFrame = {REQ_READ};
    append32(readFrame, -1); // negative out of range
    appendString(readFrame, "XXXX");
    std::vector<BYTE8> padded = padFrame(readFrame);
    sendFrame(padded);

    std::vector<BYTE8> response = readResponseFrame();
    checkResponseFrameTag(response, RES_BADID);
    checkResponseFrameSize(response, 4); // RES_BADID + 0 + 0 + 0-pad
    EXPECT_EQ((int)response[3], 0x00);
    EXPECT_EQ((int)response[4], 0x00);
}

TEST_F(TestProtocolHandler, ReadFromPositiveOutOfRangeFileIsUnsuccessful)
{
    std::vector<BYTE8> readFrame = {REQ_READ};
    append32(readFrame, MAX_FILES); // positive out of range
    appendString(readFrame, "XXXX");
    std::vector<BYTE8> padded = padFrame(readFrame);
    sendFrame(padded);

    std::vector<BYTE8> response = readResponseFrame();
    checkResponseFrameTag(response, RES_BADID);
    checkResponseFrameSize(response, 4); // RES_BADID + 0 + 0 + 0-pad
    EXPECT_EQ((int)response[3], 0x00);
    EXPECT_EQ((int)response[4], 0x00);
}

TEST_F(TestProtocolHandler, ReadOk)
{
    // Redirect stdin stream to a stringstream... the REQ_READ will read from there...
    std::stringstream stringstream("ABCD");
    std::streambuf *buffer = stringstream.rdbuf();
    const int inputStreamId = 0;
    stubPlatform._setStreamBuf(inputStreamId, buffer);

    // Now REQ_READ...
    std::vector<BYTE8> readFrame = {REQ_READ};
    append32(readFrame, FILE_STDIN);
    append16(readFrame, 4);
    std::vector<BYTE8> padded = padFrame(readFrame);
    sendFrame(padded);

    std::vector<BYTE8> response = readResponseFrame();
    checkResponseFrameTag(response, RES_SUCCESS);
    checkResponseFrameSize(response, 8); // RES_SUCCESS + 4 + 0 + ABCD + 0-pad
    EXPECT_EQ((int)response[3], 0x04);
    EXPECT_EQ((int)response[4], 0x00);

    // Expect redirected data...
    EXPECT_EQ((int)response[5], 'A');
    EXPECT_EQ((int)response[6], 'B');
    EXPECT_EQ((int)response[7], 'C');
    EXPECT_EQ((int)response[8], 'D');
}

// The read before write and write before read tests (one of them) was failing to read from a file (a real
// file, not a STDIN stream buffer hack. So, write this test to test that the real world works.
TEST_F(TestProtocolHandler, ReadOkFromRealFile)
{
    const std::pair<std::string, std::string> &testFilePathAndName = createRandomTempFilePathContaining("ABCD");
    const std::string &testFileName = testFilePathAndName.second;

    // Open (relative to the protocol handler's root dir, i.e. in tests, the tempdir())
    std::vector<BYTE8> openFrame = {REQ_OPEN};
    appendString(openFrame, testFileName);
    append8(openFrame, REQ_OPEN_TYPE_BINARY);
    append8(openFrame, REQ_OPEN_MODE_INPUT);
    padAndSendFrame(openFrame);

    const std::vector<BYTE8> &openResponse = readResponseFrame();
    checkResponseFrameTag(openResponse, RES_SUCCESS);
    WORD32 streamId = get32(openResponse, 3);

    // Now REQ_READ...
    std::vector<BYTE8> readFrame = {REQ_READ};
    append32(readFrame, streamId);
    append16(readFrame, 4);
    padAndSendFrame(readFrame);

    std::vector<BYTE8> response = readResponseFrame();
    checkResponseFrameTag(response, RES_SUCCESS);
    checkResponseFrameSize(response, 8); // RES_SUCCESS + 4 + 0 + ABCD + 0-pad
    EXPECT_EQ((int)response[3], 0x04);
    EXPECT_EQ((int)response[4], 0x00);

    // Expect data from file...
    EXPECT_EQ((int)response[5], 'A');
    EXPECT_EQ((int)response[6], 'B');
    EXPECT_EQ((int)response[7], 'C');
    EXPECT_EQ((int)response[8], 'D');
}

TEST_F(TestProtocolHandler, ReadTruncated)
{
    // Redirect stdin stream from a membuf... the REQ_READ will read from there, but only the first 3 bytes...
    uint8_t buf[] = { 'A', 'B', 'C', 'D' };
    membuf mbuf(buf, 3);
    const int inputStreamId = 0;
    stubPlatform._setStreamBuf(inputStreamId, &mbuf);

    // Now REQ_READ...
    std::vector<BYTE8> readFrame = {REQ_READ};
    append32(readFrame, FILE_STDIN);
    append16(readFrame, 4); // what we want and what we get won't be the same
    std::vector<BYTE8> padded = padFrame(readFrame);
    sendFrame(padded);

    std::vector<BYTE8> response = readResponseFrame();
    EXPECT_EQ(response.size(), 8);
    checkResponseFrameTag(response, RES_SUCCESS);
    checkResponseFrameSize(response, 6); // RES_SUCCESS + 3 + 0 + ABC (even: => no 0-pad)
    EXPECT_EQ((int)response[3], 0x03);
    EXPECT_EQ((int)response[4], 0x00);

    // Expect redirected data...
    EXPECT_EQ((int)response[5], 'A');
    EXPECT_EQ((int)response[6], 'B');
    EXPECT_EQ((int)response[7], 'C');
}

TEST_F(TestProtocolHandler, ReadAfterWriteFails)
{
    const std::pair<std::string, std::string> &testFilePathAndName = createRandomTempFilePathContaining();
    const std::string &testFileName = testFilePathAndName.second;

    // Open
    std::vector<BYTE8> openFrame = {REQ_OPEN};
    appendString(openFrame, testFileName);
    append8(openFrame, REQ_OPEN_TYPE_BINARY);
    append8(openFrame, REQ_OPEN_MODE_NEW_UPDATE);
    padAndSendFrame(openFrame);

    const std::vector<BYTE8> &openResponse = readResponseFrame();
    checkResponseFrameTag(openResponse, RES_SUCCESS);
    WORD32 streamId = get32(openResponse, 3);

    // Write to file...
    logInfo("Writing to file");
    std::vector<BYTE8> writeFrame = {REQ_WRITE};
    append32(writeFrame, streamId);
    appendString(writeFrame, "ZZXX");
    padAndSendFrame(writeFrame);

    checkResponseFrameTag(readResponseFrame(), RES_SUCCESS);
    // Internally, the lastop has been set to WRITE - cannot be directly sensed.

    // Now Read from file, should fail as last IO operation was write.
    logInfo("Reading from file");
    std::vector<BYTE8> readFrame = {REQ_READ};
    append32(readFrame, streamId);
    append16(readFrame, 4);
    padAndSendFrame(readFrame);

    std::vector<BYTE8> readResponse = readResponseFrame();
    checkResponseFrameTag(readResponse, RES_NOPOSN);
    checkResponseFrameSize(readResponse, 4); // RES_NOPOSN + 0 + 0 + 0-pad
    EXPECT_EQ((int)readResponse[3], 0x00);
    EXPECT_EQ((int)readResponse[4], 0x00);
}

// REQ_WRITE
TEST_F(TestProtocolHandler, WriteHandling)
{
    std::vector<BYTE8> writeFrame = {REQ_WRITE};
    append32(writeFrame, 3); // This stream isn't open.
    appendString(writeFrame, "XXXX");
    std::vector<BYTE8> padded = padFrame(writeFrame);
    bool wasSensedAsExitFrame = checkGoodFrame(padded);
    EXPECT_FALSE(wasSensedAsExitFrame);
    EXPECT_EQ(handler->unimplementedFrameCount(), 0L); // it is an implemented tag
}

TEST_F(TestProtocolHandler, WriteToUnopenFileIsUnsuccessful)
{
    std::vector<BYTE8> writeFrame = {REQ_WRITE};
    append32(writeFrame, 3); // This stream isn't open.
    appendString(writeFrame, "XXXX");
    std::vector<BYTE8> padded = padFrame(writeFrame);
    sendFrame(padded);

    std::vector<BYTE8> response = readResponseFrame();
    checkResponseFrameTag(response, RES_BADID);
    checkResponseFrameSize(response, 4); // RES_BADID + 0 + 0 + 0-pad
    EXPECT_EQ((int)response[3], 0x00);
    EXPECT_EQ((int)response[4], 0x00);
}

TEST_F(TestProtocolHandler, WriteToNegativeOutOfRangeFileIsUnsuccessful)
{
    std::vector<BYTE8> writeFrame = {REQ_WRITE};
    append32(writeFrame, -1); // negative out of range
    appendString(writeFrame, "XXXX");
    std::vector<BYTE8> padded = padFrame(writeFrame);
    sendFrame(padded);

    std::vector<BYTE8> response = readResponseFrame();
    checkResponseFrameTag(response, RES_BADID);
    checkResponseFrameSize(response, 4); // RES_BADID + 0 + 0 + 0-pad
    EXPECT_EQ((int)response[3], 0x00);
    EXPECT_EQ((int)response[4], 0x00);
}

TEST_F(TestProtocolHandler, WriteToPositiveOutOfRangeFileIsUnsuccessful)
{
    std::vector<BYTE8> writeFrame = {REQ_WRITE};
    append32(writeFrame, MAX_FILES); // positive out of range
    appendString(writeFrame, "XXXX");
    std::vector<BYTE8> padded = padFrame(writeFrame);
    sendFrame(padded);

    std::vector<BYTE8> response = readResponseFrame();
    checkResponseFrameTag(response, RES_BADID);
    checkResponseFrameSize(response, 4); // RES_BADID + 0 + 0 + 0-pad
    EXPECT_EQ((int)response[3], 0x00);
    EXPECT_EQ((int)response[4], 0x00);
}

TEST_F(TestProtocolHandler, WriteOk)
{
    // Redirect stdout stream to a stringstream... the REQ_WRITE will write there...
    std::stringstream stringstream;
    std::streambuf *buffer = stringstream.rdbuf();
    const int outputStreamId = 1;
    stubPlatform._setStreamBuf(outputStreamId, buffer);

    // Now REQ_WRITE...
    std::vector<BYTE8> writeFrame = {REQ_WRITE};
    append32(writeFrame, FILE_STDOUT);
    appendString(writeFrame, "ABCD");
    std::vector<BYTE8> padded = padFrame(writeFrame);
    sendFrame(padded);

    std::vector<BYTE8> response = readResponseFrame();
    checkResponseFrameTag(response, RES_SUCCESS);
    checkResponseFrameSize(response, 4); // RES_SUCCESS + 0 + 0 + 0-pad
    EXPECT_EQ((int)response[3], 0x04);
    EXPECT_EQ((int)response[4], 0x00);

    // Expect redirected data...
    EXPECT_EQ(stringstream.str(), "ABCD");
}

TEST_F(TestProtocolHandler, WriteOkToRealFile)
{
    const std::pair<std::string, std::string> &testFilePathAndName = createRandomTempFilePathContaining();
    const std::string &testFilePath = testFilePathAndName.first;
    const std::string &testFileName = testFilePathAndName.second;

    // Open
    std::vector<BYTE8> openFrame = {REQ_OPEN};
    appendString(openFrame, testFileName);
    append8(openFrame, REQ_OPEN_TYPE_BINARY);
    append8(openFrame, REQ_OPEN_MODE_OUTPUT);
    padAndSendFrame(openFrame);

    const std::vector<unsigned char> &openResponse = readResponseFrame();
    checkResponseFrameTag(openResponse, RES_SUCCESS);
    WORD32 streamId = get32(openResponse, 3);

    // Now REQ_WRITE...
    std::vector<BYTE8> writeFrame = {REQ_WRITE};
    append32(writeFrame, streamId);
    appendString(writeFrame, "ABCD");
    padAndSendFrame(writeFrame);

    std::vector<BYTE8> writeResponse = readResponseFrame();
    checkResponseFrameTag(writeResponse, RES_SUCCESS);
    checkResponseFrameSize(writeResponse, 4); // RES_SUCCESS + 0 + 0 + 0-pad
    EXPECT_EQ((int)writeResponse[3], 0x04);
    EXPECT_EQ((int)writeResponse[4], 0x00);

    // Close
    std::vector<BYTE8> closeFrame = {REQ_CLOSE};
    append32(closeFrame, streamId);
    sendFrame(padFrame(closeFrame));

    std::vector<BYTE8> closeResponse = readResponseFrame();
    checkResponseFrameTag(closeResponse, RES_SUCCESS);
    checkResponseFrameSize(closeResponse, 2); // RES_SUCCESS + 0-pad

    // Expect redirected data...
    EXPECT_EQ(readFileContents(testFilePath), "ABCD");
}

TEST_F(TestProtocolHandler, WriteTruncated)
{
    // Redirect stdout stream to a membuf... the REQ_WRITE will write there...
    uint8_t buf[] = { 0x00, 0x01, 0x02, 0x03 };
    uint8_t overflow[] = { 0xDE, 0xAD, 0xCA, 0xFE };
    membuf mbuf(buf, 3);

    const int outputStreamId = 1;
    stubPlatform._setStreamBuf(outputStreamId, &mbuf);

    // Now REQ_WRITE...
    std::vector<BYTE8> writeFrame = {REQ_WRITE};
    append32(writeFrame, FILE_STDOUT);
    appendString(writeFrame, "ABCD");
    std::vector<BYTE8> padded = padFrame(writeFrame);
    sendFrame(padded);

    std::vector<BYTE8> response = readResponseFrame();
    checkResponseFrameTag(response, RES_SUCCESS);
    checkResponseFrameSize(response, 4); // RES_SUCCESS + 0 + 0 + 0-pad
    EXPECT_EQ((int)response[3], 0x03);
    EXPECT_EQ((int)response[4], 0x00);

    // Expect redirected data...
    EXPECT_EQ(buf[0], 'A');
    EXPECT_EQ(buf[1], 'B');
    EXPECT_EQ(buf[2], 'C');
    EXPECT_EQ(buf[3], 0x03);
    EXPECT_EQ(overflow[0], 0xDE);
    EXPECT_EQ(overflow[1], 0xAD);
    EXPECT_EQ(overflow[2], 0xCA);
    EXPECT_EQ(overflow[3], 0xFE);
}

TEST_F(TestProtocolHandler, WriteZero)
{
    writesensingbuf wsbuf;
    const int outputStreamId = 1;
    stubPlatform._setStreamBuf(outputStreamId, &wsbuf);

    // Now REQ_WRITE...
    std::vector<BYTE8> writeFrame = {REQ_WRITE};
    append32(writeFrame, FILE_STDOUT);
    append16(writeFrame, 0); // no data to write
    std::vector<BYTE8> padded = padFrame(writeFrame);
    sendFrame(padded);

    std::vector<BYTE8> response = readResponseFrame();
    checkResponseFrameTag(response, RES_SUCCESS);
    checkResponseFrameSize(response, 4); // RES_SUCCESS + 0 + 0 + 0-pad
    EXPECT_EQ((int)response[3], 0x00);
    EXPECT_EQ((int)response[4], 0x00);

    // Expect no data to have been written to the write sensing buffer.
    EXPECT_EQ(wsbuf.written, false);
}

// TODO remove/inline unnecessary intermediate tests. Assert one thing per test (where possible)

TEST_F(TestProtocolHandler, WriteAfterReadFails)
{
    const std::pair<std::string, std::string> &testFilePathAndName = createRandomTempFilePathContaining("12345678");
    const std::string &testFileName = testFilePathAndName.second;

    // Open
    std::vector<BYTE8> openFrame = {REQ_OPEN};
    appendString(openFrame, testFileName);
    append8(openFrame, REQ_OPEN_TYPE_BINARY);
    append8(openFrame, REQ_OPEN_MODE_NEW_UPDATE);
    padAndSendFrame(openFrame);

    const std::vector<unsigned char> &openResponse = readResponseFrame();
    checkResponseFrameTag(openResponse, RES_SUCCESS);
    WORD32 streamId = get32(openResponse, 3);

    // Read from file...
    logInfo("Reading from file");
    std::vector<BYTE8> readFrame = {REQ_READ};
    append32(readFrame, streamId);
    append16(readFrame, 4);
    padAndSendFrame(readFrame);

    std::vector<BYTE8> readResponse = readResponseFrame();
    checkResponseFrameTag(readResponse, RES_SUCCESS);
    // WOZERE this says success but reads 0 bytes - why?
    // Internally, the lastop has been set to IO_READ - cannot be directly sensed.

    // Now Write to file, should fail as last IO operation was read.
    logInfo("Writing to file");
    std::vector<BYTE8> writeFrame = {REQ_WRITE};
    append32(writeFrame, streamId);
    appendString(writeFrame, "ABCD");
    padAndSendFrame(writeFrame);

    std::vector<BYTE8> writeResponse = readResponseFrame();
    checkResponseFrameTag(writeResponse, RES_NOPOSN);
    checkResponseFrameSize(writeResponse, 4); // RES_NOPOSN + 0 + 0 + 0-pad
    EXPECT_EQ((int)writeResponse[3], 0x00);
    EXPECT_EQ((int)writeResponse[4], 0x00);
}

// REQ_GETS

// REQ_PUTS
TEST_F(TestProtocolHandler, PutsHandling)
{
    std::vector<BYTE8> putsFrame = {REQ_PUTS};
    append32(putsFrame, 3); // This stream isn't open.
    appendString(putsFrame, "XXXX");
    std::vector<BYTE8> padded = padFrame(putsFrame);
    bool wasSensedAsExitFrame = checkGoodFrame(padded);
    EXPECT_FALSE(wasSensedAsExitFrame);
    EXPECT_EQ(handler->unimplementedFrameCount(), 0L); // it is an implemented tag
}

TEST_F(TestProtocolHandler, PutsToUnopenFileIsUnsuccessful)
{
    std::vector<BYTE8> putsFrame = {REQ_PUTS};
    append32(putsFrame, 3); // This stream isn't open.
    appendString(putsFrame, "XXXX");
    std::vector<BYTE8> padded = padFrame(putsFrame);
    sendFrame(padded);

    std::vector<BYTE8> response = readResponseFrame();
    checkResponseFrameTag(response, RES_BADID);
    checkResponseFrameSize(response, 4); // RES_BADID + 0 + 0 + 0-pad
    EXPECT_EQ((int)response[3], 0x00);
    EXPECT_EQ((int)response[4], 0x00);
}

TEST_F(TestProtocolHandler, PutsToNegativeOutOfRangeFileIsUnsuccessful)
{
    std::vector<BYTE8> putsFrame = {REQ_PUTS};
    append32(putsFrame, -1); // negative out of range
    appendString(putsFrame, "XXXX");
    std::vector<BYTE8> padded = padFrame(putsFrame);
    sendFrame(padded);

    std::vector<BYTE8> response = readResponseFrame();
    checkResponseFrameTag(response, RES_BADID);
    checkResponseFrameSize(response, 4); // RES_BADID + 0 + 0 + 0-pad
    EXPECT_EQ((int)response[3], 0x00);
    EXPECT_EQ((int)response[4], 0x00);
}

TEST_F(TestProtocolHandler, PutsToPositiveOutOfRangeFileIsUnsuccessful)
{
    std::vector<BYTE8> putsFrame = {REQ_PUTS};
    append32(putsFrame, MAX_FILES); // positive out of range
    appendString(putsFrame, "XXXX");
    std::vector<BYTE8> padded = padFrame(putsFrame);
    sendFrame(padded);

    std::vector<BYTE8> response = readResponseFrame();
    checkResponseFrameTag(response, RES_BADID);
    checkResponseFrameSize(response, 4); // RES_BADID + 0 + 0 + 0-pad
    EXPECT_EQ((int)response[3], 0x00);
    EXPECT_EQ((int)response[4], 0x00);
}

TEST_F(TestProtocolHandler, PutsOk)
{
    // Redirect stdout stream to a stringstream... the REQ_PUTS will write there...
    std::stringstream stringstream;
    std::streambuf *buffer = stringstream.rdbuf();
    const int outputStreamId = 1;
    stubPlatform._setStreamBuf(outputStreamId, buffer);

    // Now REQ_PUTS...
    std::vector<BYTE8> putsFrame = {REQ_PUTS};
    append32(putsFrame, FILE_STDOUT);
    appendString(putsFrame, "ABCD");
    std::vector<BYTE8> padded = padFrame(putsFrame);
    sendFrame(padded);

    std::vector<BYTE8> response = readResponseFrame();
    checkResponseFrameTag(response, RES_SUCCESS);
    checkResponseFrameSize(response, 2); // RES_SUCCESS + 0-pad

    // Expect redirected data...
#if defined(PLATFORM_WINDOWS)
    EXPECT_EQ(stringstream.str(), "ABCD\r\n");
#endif
#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
    EXPECT_EQ(stringstream.str(), "ABCD\n");
#endif
}

TEST_F(TestProtocolHandler, PutsOkToRealBinaryFile)
{
    const std::pair<std::string, std::string> &testFilePathAndName = createRandomTempFilePathContaining();
    const std::string &testFilePath = testFilePathAndName.first;
    const std::string &testFileName = testFilePathAndName.second;

    // Open
    std::vector<BYTE8> openFrame = {REQ_OPEN};
    appendString(openFrame, testFileName);
    append8(openFrame, REQ_OPEN_TYPE_BINARY);
    append8(openFrame, REQ_OPEN_MODE_OUTPUT);
    padAndSendFrame(openFrame);

    const std::vector<unsigned char> &openResponse = readResponseFrame();
    checkResponseFrameTag(openResponse, RES_SUCCESS);
    WORD32 streamId = get32(openResponse, 3);

    // Now REQ_PUTS...
    std::vector<BYTE8> putsFrame = {REQ_PUTS};
    append32(putsFrame, streamId);
    appendString(putsFrame, "ABCD");
    padAndSendFrame(putsFrame);

    std::vector<BYTE8> writeResponse = readResponseFrame();
    checkResponseFrameTag(writeResponse, RES_SUCCESS);
    checkResponseFrameSize(writeResponse, 2); // RES_SUCCESS + 0-pad

    // Close
    std::vector<BYTE8> closeFrame = {REQ_CLOSE};
    append32(closeFrame, streamId);
    sendFrame(padFrame(closeFrame));

    std::vector<BYTE8> closeResponse = readResponseFrame();
    checkResponseFrameTag(closeResponse, RES_SUCCESS);
    checkResponseFrameSize(closeResponse, 2); // RES_SUCCESS + 0-pad

    // Expect redirected data...
    // On Windows, REQ_PUTS on a binary or text file will add \r\n.
    // On non-Windows, it'll add \n.
#if defined(PLATFORM_WINDOWS)
    EXPECT_EQ(readFileContents(testFilePath), "ABCD\r\n");
#endif
#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
    EXPECT_EQ(readFileContents(testFilePath), "ABCD\n");
#endif
}

TEST_F(TestProtocolHandler, PutsOkToRealTextFile)
{
    const std::pair<std::string, std::string> &testFilePathAndName = createRandomTempFilePathContaining();
    const std::string &testFilePath = testFilePathAndName.first;
    const std::string &testFileName = testFilePathAndName.second;

    // Open
    std::vector<BYTE8> openFrame = {REQ_OPEN};
    appendString(openFrame, testFileName);
    append8(openFrame, REQ_OPEN_TYPE_TEXT);
    append8(openFrame, REQ_OPEN_MODE_OUTPUT);
    padAndSendFrame(openFrame);

    const std::vector<unsigned char> &openResponse = readResponseFrame();
    checkResponseFrameTag(openResponse, RES_SUCCESS);
    WORD32 streamId = get32(openResponse, 3);

    // Now REQ_PUTS...
    std::vector<BYTE8> putsFrame = {REQ_PUTS};
    append32(putsFrame, streamId);
    appendString(putsFrame, "ABCD");
    padAndSendFrame(putsFrame);

    std::vector<BYTE8> writeResponse = readResponseFrame();
    checkResponseFrameTag(writeResponse, RES_SUCCESS);
    checkResponseFrameSize(writeResponse, 2); // RES_SUCCESS + 0-pad

    // Close
    std::vector<BYTE8> closeFrame = {REQ_CLOSE};
    append32(closeFrame, streamId);
    sendFrame(padFrame(closeFrame));

    std::vector<BYTE8> closeResponse = readResponseFrame();
    checkResponseFrameTag(closeResponse, RES_SUCCESS);
    checkResponseFrameSize(closeResponse, 2); // RES_SUCCESS + 0-pad

    // Expect redirected data...
    // REQ_PUTS will add a \n to the line, and for TEXT files, the underlying C++ iostream
    // library will turn this into \r\n for Windows, and \n for non-Windows. For BINARY
    // files, Windows will add \r\n and non-Windows will add \n.
#if defined(PLATFORM_WINDOWS)
    EXPECT_EQ(readFileContents(testFilePath), "ABCD\r\n");
#endif
#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
    EXPECT_EQ(readFileContents(testFilePath), "ABCD\n");
#endif
}

TEST_F(TestProtocolHandler, PutsTruncated)
{
    // Redirect stdout stream to a membuf... the REQ_PUTS will write there...
    uint8_t buf[] = { 0x00, 0x01, 0x02, 0x03 };
    uint8_t overflow[] = { 0xDE, 0xAD, 0xCA, 0xFE };
    membuf mbuf(buf, 4);

    const int outputStreamId = 1;
    stubPlatform._setStreamBuf(outputStreamId, &mbuf);

    // If this was a stream we opened, we control its TEXT/BINARY nature. But this is stdout;
    // it's binary by default. You wouldn't usually call REQ_PUTS on a binary file, but if
    // you do, you'll get the right platform line ending.

    // Now REQ_WRITE...
    std::vector<BYTE8> putsFrame = {REQ_PUTS};
    append32(putsFrame, FILE_STDOUT);
    appendString(putsFrame, "AB");
    std::vector<BYTE8> padded = padFrame(putsFrame);
    sendFrame(padded);

    std::vector<BYTE8> response = readResponseFrame();
    checkResponseFrameTag(response, RES_SUCCESS);
    checkResponseFrameSize(response, 2); // RES_SUCCESS + 0-pad

    // Expect redirected data...
#if defined(PLATFORM_WINDOWS)
    EXPECT_EQ(buf[0], 'A');
    EXPECT_EQ(buf[1], 'B');
    EXPECT_EQ(buf[2], '\r');
    EXPECT_EQ(buf[3], '\n');
#endif
#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
    EXPECT_EQ(buf[0], 'A');
    EXPECT_EQ(buf[1], 'B');
    EXPECT_EQ(buf[2], '\n');
    EXPECT_EQ(buf[3], 0x03);
#endif
    EXPECT_EQ(overflow[0], 0xDE);
    EXPECT_EQ(overflow[1], 0xAD);
    EXPECT_EQ(overflow[2], 0xCA);
    EXPECT_EQ(overflow[3], 0xFE);
}

TEST_F(TestProtocolHandler, PutsZero)
{
    // Redirect stdout stream to a membuf... the REQ_PUTS will write there...
    uint8_t buf[] = { 0x00, 0x01, 0x02, 0x03 };
    uint8_t overflow[] = { 0xDE, 0xAD, 0xCA, 0xFE };
    membuf mbuf(buf, 3);
    const int outputStreamId = 1;
    stubPlatform._setStreamBuf(outputStreamId, &mbuf);

    // Now REQ_PUTS...
    std::vector<BYTE8> putsFrame = {REQ_PUTS};
    append32(putsFrame, FILE_STDOUT);
    append16(putsFrame, 0); // no data to write
    std::vector<BYTE8> padded = padFrame(putsFrame);
    sendFrame(padded);

    std::vector<BYTE8> response = readResponseFrame();
    checkResponseFrameTag(response, RES_SUCCESS);
    checkResponseFrameSize(response, 2); // RES_SUCCESS + 0-pad

    // Expect only the platform-specific line end to have been written to the write sensing buffer.
#if defined(PLATFORM_WINDOWS)
    EXPECT_EQ(buf[0], '\r');
    EXPECT_EQ(buf[1], '\n');
    EXPECT_EQ(buf[2], 0x02);
    EXPECT_EQ(buf[3], 0x03);
#endif
#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
    EXPECT_EQ(buf[0], '\n');
    EXPECT_EQ(buf[1], 0x01);
    EXPECT_EQ(buf[2], 0x02);
    EXPECT_EQ(buf[3], 0x03);
#endif
    EXPECT_EQ(overflow[0], 0xDE);
    EXPECT_EQ(overflow[1], 0xAD);
    EXPECT_EQ(overflow[2], 0xCA);
    EXPECT_EQ(overflow[3], 0xFE);
}

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

TEST_F(TestProtocolHandler, GetKey)
{
    // Redirect stdin stream from a membuf... the REQ_GETKEY will read from there, a single byte.
    uint8_t buf[] = { 'A' };
    membuf mbuf(buf, 1);
    const int inputStreamId = 0;
    stubPlatform._setStreamBuf(inputStreamId, &mbuf);

    // Now REQ_GETKEY...
    std::vector<BYTE8> getKeyFrame = {REQ_GETKEY};
    std::vector<BYTE8> padded = padFrame(getKeyFrame);
    bool wasSensedAsExitFrame = checkGoodFrame(padded);
    EXPECT_FALSE(wasSensedAsExitFrame);
    EXPECT_EQ(handler->unimplementedFrameCount(), 0L); // it is an implemented tag

    std::vector<BYTE8> response = readResponseFrame();
    EXPECT_EQ(response.size(), 4);
    checkResponseFrameTag(response, RES_SUCCESS);
    checkResponseFrameSize(response, 2); // RES_SUCCESS + A (no padding)
    EXPECT_EQ((int)response[3], 0x41);
}

// REQ_POLLKEY

TEST_F(TestProtocolHandler, PollKeyNothingAvailable)
{
    // Do not 'press any keys'

    // Now REQ_POLLKEY...
    std::vector<BYTE8> pollKeyFrame = {REQ_POLLKEY};
    std::vector<BYTE8> padded = padFrame(pollKeyFrame);
    bool wasSensedAsExitFrame = checkGoodFrame(padded);
    EXPECT_FALSE(wasSensedAsExitFrame);
    EXPECT_EQ(handler->unimplementedFrameCount(), 0L); // it is an implemented tag

    std::vector<BYTE8> response = readResponseFrame();
    EXPECT_EQ(response.size(), 4);
    checkResponseFrameTag(response, RES_ERROR);
    checkResponseFrameSize(response, 2); // RES_ERROR + padding
}

TEST_F(TestProtocolHandler, PollKeySomethingAvailable)
{
    // Press A key..
    stubPlatform.putKeyboardChar('A');

    // Now REQ_POLLKEY...
    std::vector<BYTE8> pollKeyFrame = {REQ_POLLKEY};
    std::vector<BYTE8> padded = padFrame(pollKeyFrame);
    bool wasSensedAsExitFrame = checkGoodFrame(padded);
    EXPECT_FALSE(wasSensedAsExitFrame);
    EXPECT_EQ(handler->unimplementedFrameCount(), 0L); // it is an implemented tag

    std::vector<BYTE8> response = readResponseFrame();
    EXPECT_EQ(response.size(), 4);
    checkResponseFrameTag(response, RES_SUCCESS);
    checkResponseFrameSize(response, 2); // RES_SUCCESS + A (no padding)
    EXPECT_EQ((int)response[3], 0x41);
}

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

TEST_F(TestProtocolHandler, CommandFrameFull)
{
    stubPlatform.setCommandLines("is a b c", "a b c");
    std::vector<BYTE8> cmdFrame = {REQ_COMMAND};
    append8(cmdFrame, 1);
    std::vector<BYTE8> padded = padFrame(cmdFrame);
    EXPECT_EQ(checkGoodFrame(padded), false); // its length is good, it's not an exit frame
    EXPECT_EQ(handler->unimplementedFrameCount(), 0L); // it is an implemented tag
    std::vector<BYTE8> response = readResponseFrame();
    checkResponseFrameSize(response, 12); // 11 with 1 byte padding
    checkResponseFrameTag(response, RES_SUCCESS);
    EXPECT_EQ((int)response[3], 0x08);
    EXPECT_EQ((int)response[4], 0x00);
    EXPECT_EQ((int)response[5], 'i');
    EXPECT_EQ((int)response[6], 's');
    EXPECT_EQ((int)response[7], ' ');
    EXPECT_EQ((int)response[8], 'a');
    EXPECT_EQ((int)response[9], ' ');
    EXPECT_EQ((int)response[10], 'b');
    EXPECT_EQ((int)response[11], ' ');
    EXPECT_EQ((int)response[12], 'c');
}

TEST_F(TestProtocolHandler, CommandFrameForProgram)
{
    stubPlatform.setCommandLines("is a b c", "a b c");
    std::vector<BYTE8> cmdFrame = {REQ_COMMAND};
    append8(cmdFrame, 0);
    std::vector<BYTE8> padded = padFrame(cmdFrame);
    EXPECT_EQ(checkGoodFrame(padded), false); // its length is good, it's not an exit frame
    EXPECT_EQ(handler->unimplementedFrameCount(), 0L); // it is an implemented tag
    std::vector<BYTE8> response = readResponseFrame();
    checkResponseFrameSize(response, 8);
    checkResponseFrameTag(response, RES_SUCCESS);
    EXPECT_EQ((int)response[3], 0x05);
    EXPECT_EQ((int)response[4], 0x00);
    EXPECT_EQ((int)response[5], 'a');
    EXPECT_EQ((int)response[6], ' ');
    EXPECT_EQ((int)response[7], 'b');
    EXPECT_EQ((int)response[8], ' ');
    EXPECT_EQ((int)response[9], 'c');
}


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

// DevZendo.org Extended IServer requests
// REQ_PUTCHAR
TEST_F(TestProtocolHandler, PutCharHandling)
{
    std::vector<BYTE8> putsFrame = {REQ_PUTCHAR};
    append8(putsFrame, 0x30);
    std::vector<BYTE8> padded = padFrame(putsFrame);
    bool wasSensedAsExitFrame = checkGoodFrame(padded);
    EXPECT_FALSE(wasSensedAsExitFrame);
    EXPECT_EQ(handler->unimplementedFrameCount(), 0L); // it is an implemented tag
}

TEST_F(TestProtocolHandler, PutCharOk)
{
    // Redirect stdout stream to a stringstream... the REQ_PUTCHAR will write there...
    std::stringstream stringstream;
    std::streambuf *buffer = stringstream.rdbuf();
    const int outputStreamId = 1;
    stubPlatform._setStreamBuf(outputStreamId, buffer);

    // Now REQ_PUTCHAR...
    std::vector<BYTE8> putsFrame = {REQ_PUTCHAR};
    append8(putsFrame, 0x30);
    std::vector<BYTE8> padded = padFrame(putsFrame);
    sendFrame(padded);

    std::vector<BYTE8> response = readResponseFrame();
    checkResponseFrameTag(response, RES_SUCCESS);
    checkResponseFrameSize(response, 2); // RES_SUCCESS + 0-pad

    // Expect redirected data...
    EXPECT_EQ(stringstream.str(), "0");
}

