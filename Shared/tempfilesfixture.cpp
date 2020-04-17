//------------------------------------------------------------------------------
//
// File        : tempfilesfixture.cpp
// Description : Mixin class for tests to handle temporary files
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 12/02/2020
//
// (C) 2005-2020 Matt J. Gumbley
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
    unsigned long millisecondsSinceEpoch() {
        unsigned long milliseconds_since_epoch =
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

std::string TestTempFiles::createRandomTempFilePath() {
    return pathJoin(tempdir(), createRandomTempFileName());
}

std::string TestTempFiles::createRandomTempFile(const std::string &contents) {
    std::string randomTempFilePath = createRandomTempFilePath();
    createTempFile(randomTempFilePath, contents);
    return randomTempFilePath;
}

// We open files in binary so we can be assured there will be no translation shenanigans between platforms.
std::string TestTempFiles::readFileContents(const std::string &file) {
    std::ifstream fstream(file, std::fstream::in | std::fstream::binary);
    std::stringstream buffer;
    buffer << fstream.rdbuf();
    return buffer.str();
}
