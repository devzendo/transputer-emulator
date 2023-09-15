//------------------------------------------------------------------------------
//
// File        : nulllink.h
// Description : A Link that can just sends nulls
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 13/09/2023
//
// (C) 2005-2023 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _NULLLINK_H
#define _NULLLINK_H

#include <exception>
#include <queue>
#include <vector>

#include "types.h"
#include "link.h"

class NullLink : public Link {
public:
    NullLink(int linkNo, bool isServer);
    void initialise(void) throw (std::exception);
    ~NullLink(void);
    BYTE8 readByte(void) throw (std::exception);
    void writeByte(BYTE8 b) throw (std::exception);
    void resetLink(void) throw (std::exception);
    int getLinkType(void);
private:
    WORD32 myWriteSequence, myReadSequence;
};

#endif // _NULLLINK_H
