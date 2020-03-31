//------------------------------------------------------------------------------
//
// File        : testtempfiles.cpp
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
#include <fstream>
#include "testtempfiles.h"
#include "log.h"
#include "misc.h"

void TestTempFiles::removeTempFiles() {
    for (const std::string& fullPath : createdTempFiles) {
        logDebugF("removeTempFiles removing '%s'", fullPath.c_str());
        if (platform_unlink(fullPath.c_str()) == -1) {
            logErrorF("Could not delete temporary file '%s' used in test: %s", fullPath.c_str(), getLastError().c_str());
        }
    }
}

void TestTempFiles::createTempFile(const std::string &tempFile, const std::string &contents) {
    std::fstream fstream(tempFile, std::fstream::out | std::fstream::trunc);
    fstream << contents;
    fstream.flush();
    fstream.close();
    createdTempFiles.push_back(tempFile);
}

std::string TestTempFiles::readFileContents(const std::string &file) {
    std::ifstream fstream(file, std::fstream::in);
    std::stringstream buffer;
    buffer << fstream.rdbuf();
    return buffer.str();
}
