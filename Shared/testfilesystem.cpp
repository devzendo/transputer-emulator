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

#include "gtest/gtest.h"
#include "platformdetection.h"
#include "filesystem.h"
#include "log.h"
#include "gsl/gsl-lite.hpp"

TEST(TempDirectory, CanBeDiscovered)
{
    const std::string &dir = tempdir();
    logInfoF("The temp directory is [%s]", dir.c_str());
}

#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)

TEST(TempDirectory, ThrowsIfTMPDIRBad)
{
    auto tmpdir = std::getenv("TMPDIR");
//    try {
//
//    }

    //const std::string &dir = tempdir();
    //logInfoF("The temp directory is [%s]", dir.c_str());
}

#endif
