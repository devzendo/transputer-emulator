//------------------------------------------------------------------------------
//
// File        : tvslink.h
// Description : A Link that's only used by the emulator to send program and
//               input data to itself, and for the running program to send
//               output data to the output file.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 13/09/2023
//
// (C) 2005-2023 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _TVSLINK_H
#define _TVSLINK_H

#include <exception>
#include <string>
#include <fstream>
#include <iostream>

#include "types.h"
#include "link.h"

class TVSLink : public Link {
public:
    TVSLink(int linkNo, std::string tvsProgram, std::string tvsInput, std::string tvsOutput);
    void initialise(void) throw (std::exception);
    ~TVSLink(void);
    BYTE8 readByte(void) throw (std::exception);
    void writeByte(BYTE8 b) throw (std::exception);
    void resetLink(void) throw (std::exception);
    int getLinkType(void);
private:
    std::string myTVSProgram;
    std::ifstream myTVSProgramStream;
    std::string myTVSInput;
    std::ifstream myTVSInputStream;
    std::string myTVSOutput;
    std::ofstream myTVSOutputStream;
    WORD32 myWriteSequence, myReadSequence;
};

#endif // _TVSLINK_H
