//------------------------------------------------------------------------------
//
// File        : fileio.h
// Description : Encapsulate access to all active files, stdin/out/err.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 13/11/2019
//
// (C) 2005-2023 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef TRANSPUTER_EMULATOR_FILEIO_H
#define TRANSPUTER_EMULATOR_FILEIO_H

#include <exception>

class FileIO {
public:

private:
    File[MAX_FILES] myFiles;
};
#endif //TRANSPUTER_EMULATOR_FILEIO_H
