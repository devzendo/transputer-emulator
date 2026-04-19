//------------------------------------------------------------------------------
//
// File        : boot.cpp
// Description : Handles the peek/poke/boot protocol over the links to the
//               memory.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 19/04/2026
//
// (C) 2005-2026 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

// using namespace std;

#include "boot.h"

Boot::Boot() = default;

Boot::~Boot() = default;

bool Boot::initialise(Memory *memory, Link *links[4]) {
    myMemory = memory;
    for (int i = 0; i < 4; i++) {
        myLinks[i] = links[i];
    }
    return true;
}

void Boot::start() {

}
