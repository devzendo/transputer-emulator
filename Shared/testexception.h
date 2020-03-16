//------------------------------------------------------------------------------
//
// File        : testexception.h
// Description : Exception helper routines
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 16/03/2020
//
// (C) 2005-2020 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _TESTEXCEPTION_H
#define _TESTEXCEPTION_H

#include "gtest/gtest.h"

// Thanks to Quuxplusone in https://github.com/google/googletest/issues/952
#define EXPECT_THROW_WITH_MESSAGE(stmt, etype, whatstring) EXPECT_THROW( \
        try { \
            stmt; \
        } catch (const etype& ex) { \
            EXPECT_EQ(std::string(ex.what()), whatstring); \
            throw; \
        } \
    , etype)


#endif // _TESTEXCEPTION_H
