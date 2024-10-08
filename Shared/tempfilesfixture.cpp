//------------------------------------------------------------------------------
//
// File        : tempfilesfixture.cpp
// Description : Mixin class for tests to handle temporary files
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 12/02/2020
//
// (C) 2005-2024 Matt J. Gumbley
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
#include <random>
#include <chrono>
#include <fstream>
#include "tempfilesfixture.h"
#include "log.h"
#include "misc.h"
#include "filesystem.h"

namespace {
    auto millisecondsSinceEpoch() {
        auto milliseconds_since_epoch =
                std::chrono::system_clock::now().time_since_epoch() /
                std::chrono::milliseconds(1);
        return milliseconds_since_epoch;
    }

    std::minstd_rand randomGenerator(millisecondsSinceEpoch());
}

void TestTempFiles::removeTempFiles() {
    for (const std::string& fullPath : createdTempFiles) {
        logDebugF("removeTempFiles removing '%s'", fullPath.c_str());
        if (platform_unlink(fullPath.c_str()) == -1) {
            logErrorF("Could not delete temporary file '%s' used in test: %s", fullPath.c_str(), getLastError().c_str());
        }
    }
}

// We create files in binary so we can be assured there will be no translation shenanigans between platforms.
void TestTempFiles::createTempFile(const std::string &tempFile, const std::string &contents) {
    logDebugF("Creating temp file %s with contents '%s'", tempFile.c_str(), contents.c_str());
    std::fstream fstream(tempFile, std::fstream::out | std::fstream::binary | std::fstream::trunc);
    fstream << contents;
    fstream.flush();
    fstream.close();
    createdTempFiles.push_back(tempFile);
}

std::string TestTempFiles::createRandomTempFileName() {
    std::stringstream ss;
    ss << "testfile";
    ss << randomGenerator();
    ss << ".txt";
    return ss.str();
}

// Returns a path to a randomly named file under the temp dir.
std::string TestTempFiles::createRandomTempFilePath() {
    return pathJoin(tempdir(), createRandomTempFileName());
}

std::string TestTempFiles::createRandomTempFile(const std::string &contents) {
    std::string randomTempFilePath = createRandomTempFilePath();
    createTempFile(randomTempFilePath, contents);
    return randomTempFilePath;
}

// Returns the path and the name of a randomly named file under the temp dir, populated with contents.
std::pair<std::string, std::string> TestTempFiles::createRandomTempFilePathContaining(const std::string &contents) {
    const std::string &testFileName = createRandomTempFileName();
    const std::string &tempDir = tempdir();
    const std::string &testFilePath = pathJoin(tempDir, testFileName);
    createTempFile(testFilePath, contents);
    return std::pair<std::string, std::string>(testFilePath, testFileName);
}

// We open files in binary so we can be assured there will be no translation shenanigans between platforms.
std::string TestTempFiles::readFileContents(const std::string &file) {
    std::ifstream fstream(file, std::fstream::in | std::fstream::binary);
    std::stringstream buffer;
    buffer << fstream.rdbuf();
    return buffer.str();
}
