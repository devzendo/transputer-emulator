//------------------------------------------------------------------------------
//
// File        : testcharacterisation.cpp
// Description : Various OS / C++ characterisation tests.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 03/04/2020
//
// (C) 2005-2023 Matt J. Gumbley
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
    std::string writeHelloWorldWithParticularLineEndings(const char* toWrite) {
        std::string testFileName = createRandomTempFileName();
        std::string testFilePath = pathJoin(tempdir(), testFileName);
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
TEST_F(TestCharacterisation, CharacteriseCPlusPlusTextHandlingOnWindowsLFtoCRLF)
{
    std::string testFilePath = writeHelloWorldWithParticularLineEndings("hello\nworld\n");
    EXPECT_EQ(readFileContents(testFilePath), "hello\r\nworld\r\n");
}
TEST_F(TestCharacterisation, CharacteriseCPlusPlusTextHandlingOnWindowsCRLFtoCRLF)
{
    std::string testFilePath = writeHelloWorldWithParticularLineEndings("hello\r\nworld\r\n");
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

