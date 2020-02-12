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
#include <cstdlib>
#include <fstream>

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

    void createTempFile(const std::string &tempFile, const std::string &contents = "test file") {
        std::fstream fstream(tempFile, std::fstream::out | std::fstream::trunc);
        fstream << contents;
        fstream.flush();
        fstream.close();
        fullPathsOfFilesToRemoval.push_back(tempFile);
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

    // this tests _that_ the expected exception is thrown
    EXPECT_THROW({
         try {
            logInfoF("The temp dir is %s", tempdir().c_str()); // doesn't print
         }
         catch (const std::runtime_error &e)  {
             // and this tests that it has the correct message
             const std::string &expectedMessage =
                     "Could not obtain file status of temp directory " + nonExistentDir + ": No such file or directory";
             EXPECT_STREQ(expectedMessage.c_str(), e.what());
             throw;
         }
     }, std::runtime_error);
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

    // this tests _that_ the expected exception is thrown
    EXPECT_THROW({
         try {
             logInfoF("The temp dir is %s", tempdir().c_str()); // doesn't print
         }
         catch (const std::runtime_error &e)  {
             // and this tests that it has the correct message
             const std::string &expectedMessage =
                     "The 'temp directory' " + fileNotDirectory + " is not a directory";
             EXPECT_STREQ(expectedMessage.c_str(), e.what());
             throw;
         }
     }, std::runtime_error);
}

#endif
