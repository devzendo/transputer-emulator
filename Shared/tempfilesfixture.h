//------------------------------------------------------------------------------
//
// File        : tempfilesfixture.h
// Description : Mixin class for tests to handle temporary files
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 12/02/2020
//
// (C) 2005-2023 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _TEMPFILESFIXTURE_H
#define _TEMPFILESFIXTURE_H

#include <string>
#include <vector>

class TestTempFiles {
public:
    void removeTempFiles();

    void createTempFile(const std::string &tempFile, const std::string &contents = "");
    std::string createRandomTempFileName();
    std::string createRandomTempFilePath();
    std::string createRandomTempFile(const std::string &contents = "");
    std::pair<std::string, std::string> createRandomTempFilePathContaining(const std::string &contents = "");
    std::string readFileContents(const std::string &file);
private:
    std::vector<std::string> createdTempFiles;
};

#endif // _TEMPFILESFIXTURE_H
