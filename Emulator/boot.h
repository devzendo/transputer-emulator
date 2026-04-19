//------------------------------------------------------------------------------
//
// File        : boot.h
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

#ifndef BOOT_H
#define BOOT_H

#include "memory.h"
#include "link.h"

class Boot {
public:
    // 2-phase CTOR since there's only one global Boot
    Boot();
    bool initialise(Memory *memory, Link *links[4]);
    void start();
    ~Boot();
private:
    Memory *myMemory{};
    Link *myLinks[4]{};
};



#endif // BOOT_H