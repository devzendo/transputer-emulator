//------------------------------------------------------------------------------
//
// File        : test.cpp
// Description : Tests for shared code
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 27/06/2018
//
// (C) 2005-2018 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <stdio.h>
#include "types.h"

void typeSizes() {
    printf("* typeSizes\n");
    if (sizeof(WORD16) != 2) {
        printf("WORD16 is not 2 bytes long\n");
    }
    if (sizeof(WORD32) != 4) {
        printf("WORD32 is not 4 bytes long\n");
    }
    if (sizeof(SWORD32) != 4) {
        printf("SWORD32 is not 4 bytes long\n");
    }
    if (sizeof(WORD64) != 8) {
        printf("WORD64 is not 8 bytes long\n");
    }
    if (sizeof(BYTE) != 1) {
        printf("BYTE is not 1 byte long\n");
    }
}

int main(int argc, char *argv[]) {
    typeSizes();
}
