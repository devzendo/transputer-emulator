//------------------------------------------------------------------------------
//
// File        : testfilesystem.cpp
// Description : Tests for filesystem routines
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 29/01/2020
//
// (C) 2005-2020 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <unistd.h>
#include "gtest/gtest.h"
#include "platformdetection.h"
#include "filesystem.h"
#include "log.h"
#include "gsl/gsl-lite.hpp"
#include "misc.h"

class FilesystemTest : public ::testing::Test {
protected:

    void SetUp() override {
        setLogLevel(LOGLEVEL_DEBUG);
        logDebug("SetUp start");

        tempDir = tempdir();
        logFlush();

    }

    void TearDown() override {
        logDebug("TearDown start");

        for (std::string fullPath : fullPathsOfFilesToRemoval) {
            logDebugF("TearDown removing '%s'", fullPath.c_str());
            if (unlink(fullPath.c_str()) == -1) {
                logErrorF("Could not delete temporary file '%s' used in test: %s", fullPath.c_str(), getLastError().c_str());
            }
        }
        logFlush();
    }
    std::string tempDir;
    std::vector<std::string> fullPathsOfFilesToRemoval;
};


TEST_F(FilesystemTest, TempDirCanBeDiscovered)
{
    EXPECT_NE("", tempDir);
    logInfoF("The temp directory is [%s]", tempDir.c_str());
}


TEST_F(FilesystemTest, ADirectoryIsADirectory)
{
    const std::string &dir = tempdir();
    EXPECT_TRUE(pathIsDir(dir));
}


TEST_F(FilesystemTest, PathSeparatorChar)
{
#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
    EXPECT_EQ(pathSeparator, '/');
#endif
#if defined(PLATFORM_WINDOWS)
    EXPECT_EQ(pathSeparator, '\\');
#endif
}


void expectCorrectJoin(const std::string &joined) {
#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
    EXPECT_EQ(joined, "abc/def");
#endif
#if defined(PLATFORM_WINDOWS)
    EXPECT_EQ(joined, "abc\\def");
#endif
}

void expectPathSeparator(const std::string &joined) {
    std::string pathSeparatorStr{pathSeparator};
    EXPECT_EQ(joined, pathSeparatorStr);
}

TEST_F(FilesystemTest, PathJoinIsOneSeparatorIfLhsAndRhsEmpty)
{
    const std::string &joined = pathJoin("", "");
    expectPathSeparator(joined);
}

TEST_F(FilesystemTest, PathJoinStartsWithOneSeparatorIfLhsEmpty)
{
    const std::string &joined = pathJoin("", "def");
#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
    EXPECT_EQ(joined, "/def");
#endif
#if defined(PLATFORM_WINDOWS)
    EXPECT_EQ(joined, "\\def");
#endif
}

TEST_F(FilesystemTest, PathJoinEndsWithOneSeparatorIfRhsEmpty)
{
    const std::string &joined = pathJoin("abc", "");
#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
    EXPECT_EQ(joined, "abc/");
#endif
#if defined(PLATFORM_WINDOWS)
    EXPECT_EQ(joined, "abc\\");
#endif
}

TEST_F(FilesystemTest, PathJoinIncludesOneSeparator)
{
    const std::string &joined = pathJoin("abc", "def");
    expectCorrectJoin(joined);
}

TEST_F(FilesystemTest, PathJoinIncludesOneSeparatorEvenIfLhsEndsInOne)
{
#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
    const std::string &joined = pathJoin("abc/", "def");
#endif
#if defined(PLATFORM_WINDOWS)
    const std::string &joined = pathJoin("abc\\", "def");
#endif
    expectCorrectJoin(joined);
}

TEST_F(FilesystemTest, PathJoinIncludesOneSeparatorEvenIfLhsEndsInSeveral)
{
#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
    const std::string &joined = pathJoin("abc///", "def");
#endif
#if defined(PLATFORM_WINDOWS)
    const std::string &joined = pathJoin("abc\\\\\\", "def");
#endif
    expectCorrectJoin(joined);
}

TEST_F(FilesystemTest, PathJoinIncludesOneSeparatorEvenIfRhsStartsInOne)
{
#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
    const std::string &joined = pathJoin("abc", "/def");
#endif
#if defined(PLATFORM_WINDOWS)
    const std::string &joined = pathJoin("abc", "\\def");
#endif
    expectCorrectJoin(joined);
}

TEST_F(FilesystemTest, PathJoinIncludesOneSeparatorEvenIfRhsStartsInSeveral)
{
#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
    const std::string &joined = pathJoin("abc", "///def");
#endif
#if defined(PLATFORM_WINDOWS)
    const std::string &joined = pathJoin("abc", "\\\\\\def");
#endif
    expectCorrectJoin(joined);
}

TEST_F(FilesystemTest, PathJoinIncludesOneSeparatorEvenIfLhsEndsInOneAndRhsStartsInOne)
{
#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
    const std::string &joined = pathJoin("abc/", "/def");
#endif
#if defined(PLATFORM_WINDOWS)
    const std::string &joined = pathJoin("abc\\", "\\def");
#endif
    expectCorrectJoin(joined);
}

TEST_F(FilesystemTest, PathJoinIncludesOneSeparatorEvenIfLhsEndsInSeveralAndRhsStartsInSeveral)
{
#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
    const std::string &joined = pathJoin("abc///", "///def");
#endif
#if defined(PLATFORM_WINDOWS)
    const std::string &joined = pathJoin("abc\\\\\\", "\\\\\\def");
#endif
    expectCorrectJoin(joined);
}

TEST_F(FilesystemTest, PathJoinIsOneSeparatorEvenIfLhsAndRhsAreAllSeparators)
{
#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
    const std::string &joined = pathJoin("///", "///");
#endif
#if defined(PLATFORM_WINDOWS)
    const std::string &joined = pathJoin("\\\\\\", "\\\\\\");
#endif
    expectPathSeparator(joined);
}

TEST_F(FilesystemTest, PathJoinIncludesASeparatorEvenIfLhsContainsOneAndRhsContainsOne)
{
#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
    const std::string &joined = pathJoin("abc/def", "ghi/jkl");
    EXPECT_EQ(joined, "abc/def/ghi/jkl");
#endif
#if defined(PLATFORM_WINDOWS)
    const std::string &joined = pathJoin("abc\\def", "ghi\\def");
    EXPECT_EQ(joined, "abc\\def\\ghi\\jkl");
#endif
}

#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)

TEST_F(FilesystemTest, ThrowsIfTMPDIRBad)
{
    auto tmpdir = std::getenv("TMPDIR");
//    try {
//
//    }

    //const std::string &dir = tempdir();
    //logInfoF("The temp directory is [%s]", dir.c_str());
}

#endif
