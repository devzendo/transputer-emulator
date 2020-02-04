//------------------------------------------------------------------------------
//
// File        : filesystem.h
// Description : Portable filesystem routines
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 28/01/2020
//
// (C) 2005-2020 Matt J. Gumbley
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

#endif // _FILESYSTEM_H
