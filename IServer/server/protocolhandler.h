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

#include <string>
#include "types.h"
#include "link.h"
#include "platform.h"
#include "framecodec.h"

const int TransactionBufferSize = 1040;
const int StringBufferSize = TransactionBufferSize - 2 - 2 + 1; // - frame size bytes - string length bytes + \0

class ProtocolHandler {
public:
    ProtocolHandler(Link & ioLink, Platform & platform):
        myIOLink(ioLink), myPlatform(platform), bDebug(false),
        myFrameCount(0L), myBadFrameCount(0L), myUnimplementedFrameCount(0L),
        myReadFrameSize(0), myReadFrameIndex(0), myWriteFrameIndex(0), myExitCode(0) {};
    ~ProtocolHandler();
    void setDebug(bool newDebug);

    // Read a frame over the Link, decode type, call into the Platform.
    // Return true if the frame is an exit frame; false otherwise.
    bool processFrame();

    WORD64 frameCount();
    WORD64 badFrameCount();
    WORD64 unimplementedFrameCount();
    int exitCode();
private:
    bool bDebug;
    Link & myIOLink;
    Platform & myPlatform;
    FrameCodec codec;
    WORD64 myFrameCount;
    WORD64 myBadFrameCount;
    WORD64 myUnimplementedFrameCount;
    BYTE8 myTransactionBuffer[TransactionBufferSize];
    BYTE8 myStringBuffer[StringBufferSize];
    WORD16 myReadFrameSize; // set if readFrame returns true
    WORD16 myReadFrameIndex;
    WORD16 myWriteFrameIndex;
    void put(const BYTE8 byte8);
    void put(const WORD16 word16);
    void put(const WORD32 word32);
    BYTE8 get8();
    WORD16 get16();
    WORD32 get32();
    std::string getString();
    bool readFrame();
    bool requestResponse();
    void resetWriteFrame();
    bool writeFrame();
    int myExitCode;

    WORD16 fillInFrameSize();
};

#endif // _PROTOCOL_HANDLER_H