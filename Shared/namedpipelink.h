//------------------------------------------------------------------------------
//
// File        : namedpipelink.h
// Description : Implementation of link that uses a Windows named pipe.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 05/03/2019
//
// (C) 2005-2019 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _NAMEDPIPELINK_H
#define _NAMEDPIPELINK_H

#include <exception>
#include <winbase.h>

#include "types.h"
#include "link.h"

const int NAME_LEN = 256;

class NamedPipeLink : public Link {
public:
    NamedPipeLink(int linkNo, bool isServer);
    void initialise(void) throw (std::exception);
    ~NamedPipeLink(void);
    BYTE readByte(void) throw (std::exception);
    void writeByte(BYTE b) throw (std::exception);
    void resetLink(void) throw (std::exception);
    int getLinkType(void);
private:
    void connect(void) throw (std::exception);
    bool myConnected = false;
    HANDLE myPipeHandle;
    WORD32 myWriteSequence, myReadSequence;
    char myPipeName[NAME_LEN];
};

#endif // _NAMEDPIPELINK_H

