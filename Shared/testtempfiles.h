//------------------------------------------------------------------------------
//
// File        : testtempfiles.h
// Description : Mixin class for tests to handle temporary files
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 12/02/2020
//
// (C) 2005-2020 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _TESTTEMPFILES_H
#define _TESTTEMPFILES_H

#include <string>
#include <vector>

class TestTempFiles {
public:
    void removeTempFiles();

    void createTempFile(const std::string &tempFile, const std::string &contents = "");
    std::string createRandomTempFileName();
    std::string createRandomTempFilePath();
    std::string createRandomTempFile(const std::string &contents = "");
    std::string readFileContents(const std::string &file);
private:
    std::vector<std::string> createdTempFiles;
};

#endif // _TESTTEMPFILES_H
