//------------------------------------------------------------------------------
//
// File        : testmisc.cpp
// Description : Tests for miscellaneous routines
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 30/01/2020
//
// (C) 2005-2025 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include "platformdetection.h"
#if defined(PLATFORM_WINDOWS)
#include <io.h> // for _open
#endif
#include <fcntl.h>
#include "gtest/gtest.h"
#include "misc.h"

const char* foo = "foo";
double myData = 3.1415;

TEST(TestMisc, ThrowFormattedExceptionImplicitly) {
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

TEST(TestMisc, ThrowFormattedExceptionExplicitly)
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

void causeAnError() {
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
#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
        EXPECT_STREQ( "File problem: No such file or directory", e.what());
#endif
#if defined(PLATFORM_WINDOWS)
        EXPECT_STREQ( "File problem: The system cannot find the file specified.\r\n", e.what());
#endif
        throw;
    }
}

TEST(TestMisc, ThrowLastError)
{
    // this tests _that_ the expected exception is thrown
    EXPECT_THROW({
        causeAnError();
    }, std::runtime_error);
}

TEST(TestMisc, GetLastError)
{
#if defined(PLATFORM_WINDOWS)
    EXPECT_EQ(-1, _open("does-not-exist.txt", _O_RDONLY, _S_IREAD));
    EXPECT_EQ( "The system cannot find the file specified.\r\n", getLastError());
#endif
#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
    EXPECT_EQ(-1, open("does-not-exist.txt", O_RDONLY));
    EXPECT_EQ( "No such file or directory", getLastError());
#endif
}

TEST(TestMisc, StripTrailing)
{
    EXPECT_EQ("", stripTrailing('x', ""));
    EXPECT_EQ("  ", stripTrailing('x', "  "));
    EXPECT_EQ("  ", stripTrailing('x', "  x"));
    EXPECT_EQ("  ", stripTrailing('x', "  xx"));
    EXPECT_EQ("x  ", stripTrailing('x', "x  xx"));
    EXPECT_EQ("", stripTrailing('x', "x"));
}

TEST(TestMisc, StripLeading)
{
    EXPECT_EQ("", stripLeading('x', ""));
    EXPECT_EQ("  ", stripLeading('x', "  "));
    EXPECT_EQ("  ", stripLeading('x', "x  "));
    EXPECT_EQ("  ", stripLeading('x', "xx  "));
    EXPECT_EQ("  x", stripLeading('x', "xx  x"));
    EXPECT_EQ("", stripLeading('x', "x"));
}