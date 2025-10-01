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
// (C) 2005-2025 Matt J. Gumbley
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

class ProtocolHandler {
public:
    ProtocolHandler(Link & ioLink, Platform & platform, const std::string &rootDirectory):
        myIOLink(ioLink), myPlatform(platform), myRootDirectory(rootDirectory), bDebug(false),
        myFrameCount(0L), myBadFrameCount(0L), myUnimplementedFrameCount(0L),
        myExitCode(0) {};
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
    const std::string &myRootDirectory;
    FrameCodec codec;
    WORD64 myFrameCount;
    WORD64 myBadFrameCount;
    WORD64 myUnimplementedFrameCount;
    bool readFrame();
    bool requestResponse();
    bool writeFrame();
    int myExitCode;
    // Frame handling routines
    void reqOpen();
    void reqClose();
    void reqRead();
    void reqWrite();
    void reqPuts();
    void reqGetKey();
    void reqPollKey();
    void reqExit();
    void reqCommand();
    void reqId();

    // Extended frame handling routines
    void reqPutChar();
};

#endif // _PROTOCOL_HANDLER_H
