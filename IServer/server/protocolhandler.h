//------------------------------------------------------------------------------
//
// File        : protocolhandler.h
// Description : Reads bytes/words from a link, parses the IServer protocol, and
//               makes appropriate calls into the platform, sending results back
//               over the link. Repeatedly called until an exit frame is
//               received.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 12/08/2019
//
// (C) 2005-2019 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _PROTOCOL_HANDLER_H
#define _PROTOCOL_HANDLER_H

#include "types.h"
#include "link.h"
#include "platform.h"

const int TransactionBufferSize = 1040;

class ProtocolHandler {
public:
    ProtocolHandler(Link & ioLink, Platform & platform):
        myIOLink(ioLink), myPlatform(platform), bDebug(false),
        myFrameCount(0L), myBadFrameCount(0L) {};
    ~ProtocolHandler();
    void setDebug(bool newDebug);

    // Read a frame over the Link, decode type, call into the Platform.
    // Return true if the frame is an exit frame; false otherwise.
    bool processFrame();

    WORD64 frameCount();
    WORD64 badFrameCount();
private:
    bool bDebug;
    Link & myIOLink;
    Platform & myPlatform;
    WORD64 myFrameCount;
    WORD64 myBadFrameCount;
    BYTE8 myTransactionBuffer[TransactionBufferSize];
    WORD16 myFrameSize; // set if readFrame returns true
    bool readFrame(); 
    bool requestResponse();
    bool writeFrame();
};

#endif // _PROTOCOL_HANDLER_H