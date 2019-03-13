//------------------------------------------------------------------------------
//
// File        : test.cpp
// Description : Tests for shared code
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 27/06/2018
//
// (C) 2005-2019 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <stdio.h>
#include "types.h"
#include "gtest/gtest.h"

TEST(TypeSizes, WordSixteen)
{
    EXPECT_EQ(sizeof(WORD16), 2);
}

TEST(TypeSizes, WordThirtyTwo)
{
    EXPECT_EQ(sizeof(WORD32), 4);
}

TEST(TypeSizes, SignedWordThirtyTwo)
{
    EXPECT_EQ(sizeof(SWORD32), 4);
}

TEST(TypeSizes, WordSixtyFour)
{
    EXPECT_EQ(sizeof(WORD64), 8);
}

TEST(TypeSizes, Byte)
{
    EXPECT_EQ(sizeof(BYTE), 1);
}
