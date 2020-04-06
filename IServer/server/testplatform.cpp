//------------------------------------------------------------------------------
//
// File        : testplatform.cpp
// Description : Tests for the platform encapsulation
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 09/12/2019
//
// (C) 2005-2020 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include "log.h"
#include "platform.h"
#include "platformfactory.h"
#include "memstreambuf.h"
#include "filesystem.h"

#include "testtempfiles.h"
#include "testexception.h"

#include "gtest/gtest.h"

class TestPlatform : public TestTempFiles, public ::testing::Test {
protected:
    PlatformFactory *platformFactory = nullptr;
    Platform *platform = nullptr;
    BYTE8 *sampleBuf = (unsigned char *) "12345";
    std::string testFileName = "testfile.txt";
    std::string testFilePath = pathJoin(tempdir(), testFileName);

    void SetUp() override {
        setLogLevel(LOGLEVEL_DEBUG);
        platformFactory = new PlatformFactory(true);
        platform = platformFactory->createPlatform();
        platform->initialise();
        platform->setDebug(true);
    }

    void TearDown() override {
        delete platform;
        delete platformFactory;
        removeTempFiles();
    }
};

TEST_F(TestPlatform, WriteStreamToNegativeOutOfRangeStreamId)
{
    EXPECT_THROW(platform->writeStream(-1, 5, sampleBuf), std::range_error);
}

TEST_F(TestPlatform, WriteStreamToPositiveOutOfRangeStreamId)
{
    EXPECT_THROW(platform->writeStream(MAX_FILES, 5, sampleBuf), std::range_error);
}

TEST_F(TestPlatform, WriteStreamToUnallocatedStreamId)
{
    EXPECT_THROW(platform->writeStream(3, 5, sampleBuf), std::invalid_argument);
}

TEST_F(TestPlatform, iostreamRedirectTest)
{
    // Initially we read from stdin..
    std::streambuf *buf = std::cin.rdbuf();
    std::iostream iostream(buf);

    // Then we redirect from a string...
    std::stringstream stringstream("ABCD");
    std::streambuf *redirectBuffer = stringstream.rdbuf();
    iostream.rdbuf(redirectBuffer);

    // And when we read, we read the string..
    BYTE8 readBuffer[8];
    iostream.read(reinterpret_cast<char *>(readBuffer), 4);
    EXPECT_EQ(readBuffer[0], 'A');
    EXPECT_EQ(readBuffer[1], 'B');
    EXPECT_EQ(readBuffer[2], 'C');
    EXPECT_EQ(readBuffer[3], 'D');
}

TEST_F(TestPlatform, StdinCanBeReadFrom)
{
    std::stringstream stringstream("ABCD");
    std::streambuf *buffer = stringstream.rdbuf();
    const int inputStreamId = 0;
    platform->_setStreamBuf(inputStreamId, buffer);

    BYTE8 readBuffer[8];

    platform->readStream(inputStreamId, 4, readBuffer);
    EXPECT_EQ(readBuffer[0], 'A');
    EXPECT_EQ(readBuffer[1], 'B');
    EXPECT_EQ(readBuffer[2], 'C');
    EXPECT_EQ(readBuffer[3], 'D');
}

TEST_F(TestPlatform, StdoutCanBeWrittenTo)
{
    std::stringstream stringstream;
    std::streambuf *buffer = stringstream.rdbuf();
    const int outputStreamId = 1;
    platform->_setStreamBuf(outputStreamId, buffer);

    std::vector<BYTE8> shortFrame = {65, 66, 67, 68};

    platform->writeStream(outputStreamId, shortFrame.size(), shortFrame.data());
    EXPECT_EQ(stringstream.str(), "ABCD");
}

TEST_F(TestPlatform, StderrCanBeWrittenTo)
{
    std::stringstream stringstream;
    std::streambuf *buffer = stringstream.rdbuf();
    const int errorStreamId = 2;
    platform->_setStreamBuf(errorStreamId, buffer);

    std::vector<BYTE8> shortFrame = {65, 66, 67, 68};

    platform->writeStream(errorStreamId, shortFrame.size(), shortFrame.data());
    EXPECT_EQ(stringstream.str(), "ABCD");
}

TEST_F(TestPlatform, ReadStreamFromNegativeOutOfRangeStreamId)
{
    EXPECT_THROW(platform->readStream(-1, 5, sampleBuf), std::range_error);
}

TEST_F(TestPlatform, ReadStreamFromPositiveOutOfRangeStreamId)
{
    EXPECT_THROW(platform->readStream(MAX_FILES, 5, sampleBuf), std::range_error);
}

TEST_F(TestPlatform, ReadStreamToUnallocatedStreamId)
{
    EXPECT_THROW(platform->readStream(3, 5, sampleBuf), std::invalid_argument);
}

TEST_F(TestPlatform, StdinStreamCannotBeWrittenTo)
{
    EXPECT_THROW(platform->writeStream(0, 5, sampleBuf), std::runtime_error);
}

TEST_F(TestPlatform, StdoutStreamCannotBeReadFrom)
{
    EXPECT_THROW(platform->readStream(1, 5, sampleBuf), std::runtime_error);
}

TEST_F(TestPlatform, StderrStreamCannotBeReadFrom)
{
    EXPECT_THROW(platform->readStream(2, 5, sampleBuf), std::runtime_error);
}

TEST_F(TestPlatform, StreamWriteTruncated)
{
    // Redirect stdout stream to a membuf... the writeStream will write there...
    uint8_t buf[] = { 0x00, 0x01, 0x02, 0x03 };
    membuf mbuf(buf, 3);

    const int outputStreamId = 1;
    platform->_setStreamBuf(outputStreamId, &mbuf);

    std::vector<BYTE8> shortFrame = {65, 66, 67, 68};

    auto wrote = platform->writeStream(outputStreamId, shortFrame.size(), shortFrame.data());
    EXPECT_EQ(buf[0], 'A');
    EXPECT_EQ(buf[1], 'B');
    EXPECT_EQ(buf[2], 'C');
    EXPECT_EQ(buf[3], 0x03);
    EXPECT_EQ(wrote, 3);
}

TEST_F(TestPlatform, StreamReadTruncated)
{
    // Redirect stdin stream from a membuf... the readStream will read from there...
    uint8_t buf[] = { 'A', 'B', 'C', 'D' };
    membuf mbuf(buf, 3);

    const int inputStreamId = 0;
    platform->_setStreamBuf(inputStreamId, &mbuf);

    std::vector<BYTE8> readBuffer = {4, 3, 2, 1};

    auto read = platform->readStream(inputStreamId, readBuffer.size(), readBuffer.data());
    EXPECT_EQ(readBuffer[0], 'A');
    EXPECT_EQ(readBuffer[1], 'B');
    EXPECT_EQ(readBuffer[2], 'C');
    EXPECT_EQ(readBuffer[3], 0x01);
    EXPECT_EQ(read, 3);
}

TEST_F(TestPlatform, StreamWriteToStdoutFlushed) {
    flushsensingstreambuf fsbuf;

    const int outputStreamId = 1;
    platform->_setStreamBuf(outputStreamId, &fsbuf);

    std::vector<BYTE8> shortFrame = {65};

    platform->writeStream(outputStreamId, shortFrame.size(), shortFrame.data());
    EXPECT_EQ(fsbuf.flushed, true);
}

TEST_F(TestPlatform, StreamWriteToStderrFlushed) {
    flushsensingstreambuf fsbuf;

    const int outputStreamId = 2;
    platform->_setStreamBuf(outputStreamId, &fsbuf);

    std::vector<BYTE8> shortFrame = {65};

    platform->writeStream(outputStreamId, shortFrame.size(), shortFrame.data());
    EXPECT_EQ(fsbuf.flushed, true);
}

TEST_F(TestPlatform, StreamWriteToNonStdOutErrNotFlushed) {
    flushsensingfilebuf fsbuf;

    createTempFile(testFilePath, "");
    const int outputStreamId = platform->openFileStream(testFilePath, std::ios_base::out);
    EXPECT_EQ(outputStreamId, 3);
    platform->_setFileBuf(outputStreamId, fsbuf);

    std::vector<BYTE8> shortFrame = {65};

    platform->writeStream(outputStreamId, shortFrame.size(), shortFrame.data());
    EXPECT_EQ(fsbuf.flushed, false);
}

TEST_F(TestPlatform, FileOpenStreamForRead) {
    createTempFile(testFilePath, "ABCD");
    const int fileStreamId = platform->openFileStream(testFilePath, std::ios_base::in);

    std::vector<BYTE8> readBuffer = {4, 3, 2, 1}; // this'll get overwritten

    auto read = platform->readStream(fileStreamId, readBuffer.size(), readBuffer.data());
    EXPECT_EQ(readBuffer[0], 'A');
    EXPECT_EQ(readBuffer[1], 'B');
    EXPECT_EQ(readBuffer[2], 'C');
    EXPECT_EQ(readBuffer[3], 'D');
    EXPECT_EQ(read, 4);
}

TEST_F(TestPlatform, FileOpenStreamForWrite) {
    createTempFile(testFilePath, ""); // first it's empty
    const int fileStreamId = platform->openFileStream(testFilePath, std::ios_base::out);

    std::vector<BYTE8> writeBuffer = {'A', 'B', 'C', 'D'};

    auto written = platform->writeStream(fileStreamId, writeBuffer.size(), writeBuffer.data());
    // Files aren't flushed by default.. so close it.
    platform->closeStream(fileStreamId);

    EXPECT_EQ(written, 4);
    EXPECT_EQ("ABCD", readFileContents(testFilePath));
}

TEST_F(TestPlatform, NoFreeStreams) {
    createTempFile(testFilePath, "");
    // Exhaust all the streams opening the test file...
    for (int i=3; i < MAX_FILES; i++) {
        const int fileStreamId = platform->openFileStream(testFilePath, std::ios_base::in);
        EXPECT_EQ(fileStreamId, i); // just to be sure..
    }

    EXPECT_THROW_WITH_MESSAGE({
        platform->openFileStream(testFilePath, std::ios_base::in);
    }, std::runtime_error, "No streams available to open " + testFilePath);
}

TEST_F(TestPlatform, CannotWriteToAFileOpenedForReading) {
    createTempFile(testFilePath, "ABCD");
    const int fileStreamId = platform->openFileStream(testFilePath, std::ios_base::in);

    std::vector<BYTE8> writeBuffer = {4, 3, 2, 1};
    EXPECT_THROW_WITH_MESSAGE({
        platform->writeStream(fileStreamId, writeBuffer.size(), writeBuffer.data());
    }, std::runtime_error, "Stream not writable");
}

TEST_F(TestPlatform, CannotReadFromAFileOpenedForWriting) {
    createTempFile(testFilePath, "ABCD");
    const int fileStreamId = platform->openFileStream(testFilePath, std::ios_base::out);

    std::vector<BYTE8> readBuffer = {4, 3, 2, 1};
    EXPECT_THROW_WITH_MESSAGE({
        platform->readStream(fileStreamId, readBuffer.size(), readBuffer.data());
    }, std::runtime_error, "Stream not readable");
}


TEST_F(TestPlatform, CloseUnopenedStream) {
    EXPECT_THROW_WITH_MESSAGE({
          platform->closeStream(3);
    }, std::invalid_argument, "Stream id not open");
}

TEST_F(TestPlatform, CloseStreamIdOutOfRange) {
    EXPECT_THROW_WITH_MESSAGE({
          platform->closeStream(-3);
    }, std::range_error, "Stream id out of range");

}

TEST_F(TestPlatform, CloseSucceeds) {
    createTempFile(testFilePath, "");
    const int fileStreamId = platform->openFileStream(testFilePath, std::ios_base::out);

    EXPECT_EQ(platform->closeStream(fileStreamId), true);
}

TEST_F(TestPlatform, CloseFails) {
    createTempFile(testFilePath, "");
    const int fileStreamId = platform->openFileStream(testFilePath, std::ios_base::out);

    // How to get fstream close to fail for a real failure-to-close?
    // Set a streams basic_filebuf to a freshly constructed one, then its __file_ will be 0 so
    // close() returns 0, so fstream close() sets failbit
    std::basic_filebuf<char> emptiness;
    platform->_setFileBuf(fileStreamId, emptiness);
    EXPECT_EQ(platform->closeStream(fileStreamId), false);
}

