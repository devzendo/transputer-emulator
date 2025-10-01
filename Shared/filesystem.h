//------------------------------------------------------------------------------
//
// File        : filesystem.h
// Description : Portable filesystem routines
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 28/01/2020
//
// (C) 2005-2025 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _FILESYSTEM_H
#define _FILESYSTEM_H

#include <string>
#include "types.h"

// What is the system temporary directory?
extern std::string tempdir();

// Does a path refer to a directory?
extern bool pathIsDir(const std::string &path);

// The character between files and directories in a path, i.e. / or \ (windows)
extern const char pathSeparator;

// Join one part of a path to another with a single path separator in between (single path separator even if lhs ends
// in one or more of them, or rhs starts with one or more.)
extern std::string pathJoin(const std::string &lhs, const std::string &rhs);

#endif // _FILESYSTEM_H
