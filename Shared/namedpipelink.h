//------------------------------------------------------------------------------
//
// File        : namedpipelink.h
// Description : Implementation of link that uses a Windows named pipe.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 05/03/2019
//
// (C) 2005-2026 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _NAMEDPIPELINK_H
#define _NAMEDPIPELINK_H

#include <windows.h>

#include "types.h"
#include "systemlink.h"

const int NAME_LEN = 256;

class NamedPipeLink : public SystemLink {
public:
    NamedPipeLink(int linkNo, bool isServer);
    void initialise(void);
    ~NamedPipeLink(void);
    BYTE readByte(void);
    void writeByte(BYTE b);
    void resetLink(void);
    int getLinkType(void);

    bool readAvailable();
    bool writeAvailable();
    void writeFlush() override;

private:
    static constexpr int NP_MSGBUF_SIZE = 128;
    void connect(void);
    bool myConnected = false;
    HANDLE myPipeHandle;
    char myPipeName[NAME_LEN];
	char myMsgbuf[NP_MSGBUF_SIZE]{};
};

#endif // _NAMEDPIPELINK_H

