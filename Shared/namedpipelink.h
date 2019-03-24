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
using namespace std;

#include "types.h"
#include "link.h"

const int NAME_LEN = 256;

class NamedPipeLink : public Link {
public:
    NamedPipeLink(int linkNo, bool isServer);
    void initialise(void) throw (exception);
    ~NamedPipeLink(void);
    BYTE readByte(void) throw (exception);
    void writeByte(BYTE b) throw (exception);
    void resetLink(void) throw (exception);
private:
    void connect(void) throw (exception);
    bool myConnected = false;
    HANDLE myWriteHandle, myReadHandle;
    WORD32 myWriteSequence, myReadSequence;
    char myReadPipeName[NAME_LEN];
    char myWritePipeName[NAME_LEN];
};

#endif // _NAMEDPIPELINK_H

