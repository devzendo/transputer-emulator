//------------------------------------------------------------------------------
//
// File        : testcharacterisation.cpp
// Description : Various OS / C++ characterisation tests.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 03/04/2020
//
// (C) 2005-2020 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <cstdio>
#include <fstream>
#include "log.h"
#include "filesystem.h"
#include "platformdetection.h"

#include "gtest/gtest.h"
#include "testtempfiles.h"


class TestCharacterisation : public TestTempFiles, public ::testing::Test {
public:
    std::string writeHelloWorldWithNewlines() {
        std::string testFileName = createRandomTempFileName();
        std::string testFilePath = pathJoin(tempdir(), testFileName);
        std::ios::openmode iosOpenMode = (std::ios_base::out | std::ios_base::trunc);
        std::fstream fs(testFilePath, iosOpenMode);
        fs << "hello\nworld\n";
        fs.flush();
        fs.close();

        return testFilePath;
    }
};


#if defined(PLATFORM_WINDOWS)
TEST_F(TestCharacterisation, CharacteriseCPlusPlusTextHandlingOnWindows)
{
    std::string testFilePath = writeHelloWorldWithNewlines();

    EXPECT_EQ(TestTempFiles::readFileContents(testFilePath), "hello\r\nworld\r\n");
}
#endif

#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
TEST_F(TestCharacterisation, CharacteriseCPlusPlusTextHandlingOnNonWindows)
{
    std::string testFilePath = writeHelloWorldWithNewlines();
    EXPECT_EQ(readFileContents(testFilePath), "hello\nworld\n");
}
#endif

