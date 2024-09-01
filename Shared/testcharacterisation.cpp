//------------------------------------------------------------------------------
//
// File        : testcharacterisation.cpp
// Description : Various OS / C++ characterisation tests.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 03/04/2020
//
// (C) 2005-2024 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <fstream>
#include "filesystem.h"
#include "platformdetection.h"

#include "gtest/gtest.h"
#include "tempfilesfixture.h"


class TestCharacterisation : public TestTempFiles, public ::testing::Test {
public:
    std::string createTempTestFile() {
        std::string testFileName = createRandomTempFileName();
        std::string testFilePath = pathJoin(tempdir(), testFileName);

        return testFilePath;
    }
    std::string writeHelloWorldWithParticularLineEndings(const char* toWrite) {
        std::string testFilePath = createTempTestFile();
        std::ios::openmode iosOpenMode = (std::ios_base::out | std::ios_base::trunc);
        // Note: not std::ios_base::binary so it's opened in text mode.
        std::fstream fs(testFilePath, iosOpenMode);
        fs << toWrite;
        fs.flush();
        fs.close();

        return testFilePath;
    }
};


#if defined(PLATFORM_WINDOWS)
TEST_F(TestCharacterisation, CharacteriseCPlusPlusTextHandlingOnWindowsLFtoCRLFStreamInsertion)
{
    std::string testFilePath = writeHelloWorldWithParticularLineEndings("hello\nworld\n");

    EXPECT_EQ(readFileContents(testFilePath), "hello\r\nworld\r\n");
}
TEST_F(TestCharacterisation, CharacteriseCPlusPlusTextHandlingOnWindowsCRLFtoCRCRLFStreamInsertion)
{
    // This may seem weird, but it's right. CR is passed through on write, but LF is expanded
    // to CRLF. So in text mode, just write \n on any OS, and on Windows, you'll get CRLF and
    // on non-Windows, you'll get LF. ie the platform-specific line ending.
    // There are other oddities - see https://stackoverflow.com/questions/26993086/what-the-point-of-using-stdios-basebinary
    std::string testFilePath = writeHelloWorldWithParticularLineEndings("hello\r\nworld\r\n");

    EXPECT_EQ(readFileContents(testFilePath), "hello\r\r\nworld\r\r\n");
}

// Contrast this with TestPlatform::TextFileTranslation, which does exactly the same thing, but via the
// Platform API.
TEST_F(TestCharacterisation, CharacteriseCPlusPlusTextHandlingOnWindowsLFtoCRLFRdbufSputn)
{
    std::string testFilePath = createTempTestFile();
    std::ios::openmode iosOpenMode = (std::ios_base::out | std::ios_base::trunc);
    // Note: not std::ios_base::binary so it's opened in text mode.
    std::fstream fstream(testFilePath, iosOpenMode);
    // How the platform layer does it...
    std::iostream & stream = fstream;
    WORD16 written = stream.rdbuf()->sputn(reinterpret_cast<const char *>("hello\nworld\n"), 12);
    fstream.flush();
    fstream.close();

    EXPECT_EQ(written, 12);
    // And yet the file contains 14 bytes... which is "correct".
    EXPECT_EQ(readFileContents(testFilePath), "hello\r\nworld\r\n");
}
#endif

#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
TEST_F(TestCharacterisation, CharacteriseCPlusPlusTextHandlingOnNonWindows)
{
    std::string testFilePath = writeHelloWorldWithParticularLineEndings("hello\nworld\n");
    EXPECT_EQ(readFileContents(testFilePath), "hello\nworld\n");
}
#endif

TEST_F(TestCharacterisation, StdInOutErrBinaryness)
{
    bool cin_binary = std::cin.binary != 0;
    std::cout << "stdin  binaryness is " << cin_binary << std::endl;
    bool cout_binary = std::cout.binary != 0;
    std::cout << "stdout binaryness is " << cout_binary << std::endl;
    bool cerr_binary = std::cerr.binary != 0;
    std::cout << "stderr binaryness is " << cerr_binary << std::endl;
    EXPECT_EQ(cin_binary, true);
    EXPECT_EQ(cout_binary, true);
    EXPECT_EQ(cerr_binary, true);
}
