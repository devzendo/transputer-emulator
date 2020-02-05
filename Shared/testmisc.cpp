//------------------------------------------------------------------------------
//
// File        : testmisc.cpp
// Description : Tests for miscellaneous routines
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 30/01/2020
//
// (C) 2005-2020 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#if defined(PLATFORM_WINDOWS)
#include <io.h> // for _open
#endif
#include <fcntl.h>
#include "gtest/gtest.h"
#include "misc.h"

const char* foo = "foo";
double myData = 3.1415;

TEST(TempMisc, ThrowFormattedExceptionImplicitly) {
    // this tests _that_ the expected exception is thrown
    EXPECT_THROW({
        try {
            throw std::runtime_error(Formatter() << foo << 13 << ", bar" << myData);   // implicitly cast to std::string
        }
        catch (const std::runtime_error &e)  {
            // and this tests that it has the correct message
            EXPECT_STREQ("foo13, bar3.1415", e.what());
            throw;
        }
    }, std::runtime_error);
}

TEST(TempMisc, ThrowFormattedExceptionExplicitly)
{
    // this tests _that_ the expected exception is thrown
    EXPECT_THROW({
        try {
            throw std::runtime_error(Formatter() << foo << 13 << ", bar" << myData >> Formatter::to_str);    // explicitly cast to std::string
        }
        catch (const std::runtime_error &e)  {
            // and this tests that it has the correct message
            EXPECT_STREQ("foo13, bar3.1415", e.what());
            throw;
        }
    }, std::runtime_error);
}

TEST(TempMisc, ThrowLastError)
{
    // this tests _that_ the expected exception is thrown
    EXPECT_THROW({
        // Cause an error
#if defined(PLATFORM_WINDOWS)
        EXPECT_EQ(-1, _open("does-not-exist.txt", _O_RDONLY, _S_IREAD));
#endif
#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
        EXPECT_EQ(-1, open("does-not-exist.txt", O_RDONLY));
#endif

        try {
            throwLastError("File problem: ");
        }
        catch (const std::runtime_error &e)  {
            // and this tests that it has the correct message
            EXPECT_STREQ( "File problem: No such file or directory", e.what());
            throw;
        }
    }, std::runtime_error);
}

