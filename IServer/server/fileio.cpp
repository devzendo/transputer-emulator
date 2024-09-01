//------------------------------------------------------------------------------
//
// File        : fileio.cpp
// Description : Encapsulate access to all active files, stdin/out/err.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 14/11/2019
//
// (C) 2005-2024 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include "fileio.h"

#include <cstdio>

const int MAX_FILES = 128;
struct File {
    FILE *fd;

};

