//------------------------------------------------------------------------------
//
// File        : protocolhandler.cpp
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

#include "types.h"
#include "constants.h"
#include "platform.h"
#include "link.h"
#include "hexdump.h"
#include "log.h"
#include "protocolhandler.h"
#include "../isproto.h"

namespace {
    static const char* tagToName(const BYTE8 tag) {
        switch(tag) {
            case REQ_OPEN: return "Open";
            case REQ_CLOSE: return "Close";
            case REQ_READ: return "Read";
            case REQ_WRITE: return "Write";
            case REQ_GETS: return "Gets";
            case REQ_PUTS: return "Puts";
            case REQ_FLUSH: return "Flush";
            case REQ_SEEK: return "Seek";
            case REQ_TELL: return "Tell";
            case REQ_EOF: return "EOF";
            case REQ_FERROR: return "FError";
            case REQ_REMOVE: return "Remove";
            case REQ_RENAME: return "Rename";
            case REQ_GETBLOCK: return "GetBlock";
            case REQ_PUTBLOCK: return "PutBlock";
            case REQ_ISATTY: return "IsATTY";
            case REQ_OPENREC: return "OpenRec";
            case REQ_GETREC: return "GetRec";
            case REQ_PUTREC: return "PutRec";
            case REQ_PUTEOF: return "PutEOF";
            case REQ_GETKEY: return "GetKey";
            case REQ_POLLKEY: return "PollKey";
            case REQ_GETENV: return "GetEnv";
            case REQ_TIME: return "Time";
            case REQ_SYSTEM: return "System";
            case REQ_EXIT: return "Exit";
            case REQ_COMMAND: return "Command";
            case REQ_CORE: return "Core";
            case REQ_ID: return "Id";
            case REQ_GETINFO: return "GetInfo";
            case REQ_MSDOS: return "MSDOS";
            case REQ_FILEEXISTS: return "FileExists";
            case REQ_TRANSLATE: return "Translate";
            case REQ_FERRSTAT: return "FErrStat";
            case REQ_COMMANDARG: return "CommandArg";
            default: return "Unknown";
        }
    }
}

ProtocolHandler::~ProtocolHandler() {
}

void ProtocolHandler::setDebug(bool newDebug) {
    bDebug = newDebug;
}

// Read a frame over the Link, decode type, call into the Platform.
// Return true if the frame is an exit frame; false otherwise.
bool ProtocolHandler::processFrame() {
    bool exitFrameReceived = false;
    if (readFrame()) {
        exitFrameReceived = requestResponse();

        if (writeFrame()) {

        } else {

        }
    } else {

    }

    return exitFrameReceived;
}

bool ProtocolHandler::readFrame() {
    myFrameSize = 0;
    // Frame length encoded in the first two bytes (little endian).
    // Minimum frame length is 8 (ie minimum message length of 6 bytes in the to-server
    // direction), maximum 512. Packet size must always be an even number of bytes.
    // (TDS 2nd ed, p356, sec 16.5.1.)
    // Inmos iServer has max packet size of 1040 ? Is that from T9000?

    // Read 8 bytes into transaction buffer; extract frame size from first two bytes.
    // Fail if we fail to read 8 bytes.
    myFrameSize = myIOLink.readShort();
    // TODO when link abstraction is improved to add timeout reads, and analyse/reset/error
    // rework this to take those into account.
    myFrameCount++;
    if (bDebug) {
        logDebugF("Message length word is %04X (%d)", myFrameSize, myFrameSize);
    }
    if (myFrameSize < 6 || myFrameSize > 510) {
        logWarnF("Frame size %04X out of range", myFrameSize);
        myBadFrameCount++;
        return false;
    }
    if ((myFrameSize & 0x01) == 0x01) {
        logWarnF("Frame size %04X is odd", myFrameSize);
        myBadFrameCount++;
        return false;
    }
    // Fail if frame size > max frame length.
    // Store frameSize at positions 0 and 1 in myTransactionBuffer?
    // Read remaining data.
    WORD16 bytesRead = myIOLink.readBytes(&myTransactionBuffer[2], myFrameSize);
    // Fail if we fail to read all of it.
    if (bytesRead < myFrameSize) {
        logWarnF("Truncated frame read: read %d bytes, expecting %d bytes", bytesRead, myFrameSize);
        return false;
        // How to resynchronise? Reset link?
    }
    return true;
}

bool ProtocolHandler::requestResponse() {
    BYTE8 tag = myTransactionBuffer[2];
    logDebugF("Frame tag %02X (%s)", tag, tagToName(tag));
    //switch
    return tag == REQ_EXIT;
}

bool ProtocolHandler::writeFrame() {
    return true;
}

WORD64 ProtocolHandler::frameCount() {
    return myFrameCount;
}

WORD64 ProtocolHandler::badFrameCount() {
    return myBadFrameCount;
}