//------------------------------------------------------------------------------
//
// File        : testfilesystem.cpp
// Description : Tests for filesystem routines
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 29/01/2020
//
// (C) 2005-2025 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include "platformdetection.h"
#if defined(PLATFORM_WINDOWS)
#include <io.h> // for _unlink
#endif
#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
#include <unistd.h> // for unlink
#endif

#include <cstdlib>
#include <fstream>

#include "gtest/gtest.h"

#include "filesystem.h"
#include "log.h"
#include "gsl/gsl-lite.hpp"
#include "misc.h"

#include "tempfilesfixture.h"
#include "exceptionfixture.h"

class FilesystemTest : public TestTempFiles, public ::testing::Test {
protected:

    void SetUp() override {
        setLogLevel(LOGLEVEL_DEBUG);
        logDebug("SetUp start");

        tempDir = tempdir();
        logFlush();

    }

    void TearDown() override {
        logDebug("TearDown start");
        removeTempFiles();
        logFlush();
    }

    std::string tempDir;
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
    const std::string &joined = pathJoin("abc\\def", "ghi\\jkl");
    EXPECT_EQ(joined, "abc\\def\\ghi\\jkl");
#endif
}

#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)

TEST_F(FilesystemTest, ThrowsIfTMPDIRNonExistent)
{
    // Get TMPDIR and ensure it's reset after we set it to something nonexistent...
    std::string tmpdir = std::getenv("TMPDIR");
    auto _ = gsl::finally([tmpdir] {
        logInfoF("Resetting TMPDIR to %s", tmpdir.c_str());
        if (setenv("TMPDIR", tmpdir.c_str(), 1) != 0) {
            logErrorF("Could not reset TMPDIR after test: %s", getLastError().c_str());
            // finally is noexcept, can't fail
        };
    });
    
    std::string nonExistentDir = pathJoin(tmpdir, "nonexistentdir");
    logInfoF("Setting TMPDIR to %s", nonExistentDir.c_str());
    if (setenv("TMPDIR", nonExistentDir.c_str(), 1) != 0) {
        logErrorF("Could not set TMPDIR to %s: %s", nonExistentDir.c_str(), getLastError().c_str());
        FAIL();
    };

    EXPECT_THROW_WITH_MESSAGE({
            logInfoF("The temp dir is %s", tempdir().c_str()); // doesn't print
        }, std::runtime_error,
        "Could not obtain file status of temp directory " + nonExistentDir + ": No such file or directory");
}

TEST_F(FilesystemTest, ThrowsIfTMPDIRNotADirectory)
{
    // Get TMPDIR and ensure it's reset after we set it to something nonexistent...
    std::string tmpdir = std::getenv("TMPDIR");
    auto _ = gsl::finally([tmpdir] {
        logInfoF("Resetting TMPDIR to %s", tmpdir.c_str());
        if (setenv("TMPDIR", tmpdir.c_str(), 1) != 0) {
            logErrorF("Could not reset TMPDIR after test: %s", getLastError().c_str());
            // finally is noexcept, can't fail
        };
    });

    std::string fileNotDirectory = pathJoin(tmpdir, "filenotdirectory");
    createTempFile(fileNotDirectory);

    logInfoF("Setting TMPDIR to %s", fileNotDirectory.c_str());
    if (setenv("TMPDIR", fileNotDirectory.c_str(), 1) != 0) {
        logErrorF("Could not set TMPDIR to %s: %s", fileNotDirectory.c_str(), getLastError().c_str());
        FAIL();
    };

    EXPECT_THROW_WITH_MESSAGE({
             logInfoF("The temp dir is %s", tempdir().c_str()); // doesn't print
         }, std::runtime_error,
         "The 'temp directory' " + fileNotDirectory + " is not a directory");
}

#endif
