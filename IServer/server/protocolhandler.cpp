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

            case RES_SUCCESS: return "Success";
            case RES_UNIMPLEMENTED: return "Unimplement";
            case RES_ERROR: return "Error";
            case RES_NOPRIV: return "NoPriv";
            case RES_NORESOURCE: return "NoResource";
            case RES_NOFILE: return "NoFile";
            case RES_TRUNCATED: return "Truncated";
            case RES_BADID: return "BadId";
            case RES_NOPOSN: return "NoPosn";
            case RES_NOTAVAILABLE: return "NotAvailable";
            case RES_EOF: return "EOF";
            case RES_AKEYREPLY: return "AKeyReply";
            case RES_BADPARAMS: return "BadParams";
            case RES_NOTERM: return "NoTerm";
            case RES_RECTOOBIG: return "RecTooBig";

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

void ProtocolHandler::put(const BYTE8 byte8) {
    logDebugF("put BYTE8 %02X @ %04X", byte8, myWriteFrameIndex);
    myTransactionBuffer[myWriteFrameIndex++] = byte8;
}

void ProtocolHandler::put(const WORD16 word16) {
    logDebugF("put WORD16 %04X @ %04X UNFINISHED TEST", word16, myWriteFrameIndex);
    myTransactionBuffer[myWriteFrameIndex++] = (BYTE8) (word16 & (BYTE8)0xff);

// TODO how to test this?    myTransactionBuffer[myWriteFrameIndex++] = (BYTE8) (word16 >> (BYTE8)8) & (BYTE8)0xff;
    myTransactionBuffer[myWriteFrameIndex++] = (BYTE8) 0;  // BAD DATA - NEED TEST TO ESTABLISH ABOVE
}

void ProtocolHandler::put(const WORD32 word32) {
    logDebugF("put WORD32 %08X @ %04X UNIMPLEMENTD", word32, myWriteFrameIndex);

}

bool ProtocolHandler::readFrame() {
    myReadFrameSize = 0;
    // Frame length encoded in the first two bytes (little endian).
    // Minimum frame length is 8 (ie minimum message length of 6 bytes in the to-server
    // direction), maximum 512. Packet size must always be an even number of bytes.
    // (TDS 2nd ed, p356, sec 16.5.1.)
    // Inmos iServer has max packet size of 1040 ? Is that from T9000?

    // Read 8 bytes into transaction buffer; extract frame size from first two bytes.
    // Fail if we fail to read 8 bytes.
    myReadFrameSize = myIOLink.readShort();
    // TODO when link abstraction is improved to add timeout reads, and analyse/reset/error
    // rework this to take those into account.
    myFrameCount++;
    if (bDebug) {
        logDebugF("Read frame size word is %04X (%d)", myReadFrameSize, myReadFrameSize);
    }
    if (myReadFrameSize < 6 || myReadFrameSize > 510) {
        logWarnF("Read frame size %04X out of range", myReadFrameSize);
        myBadFrameCount++;
        return false;
    }
    if ((myReadFrameSize & 0x01) == 0x01) {
        logWarnF("Read frame size %04X is odd", myReadFrameSize);
        myBadFrameCount++;
        return false;
    }
    // Fail if frame size > max frame length.
    // Store frameSize at positions 0 and 1 in myTransactionBuffer?
    // Read remaining data.
    WORD16 bytesRead = myIOLink.readBytes(&myTransactionBuffer[2], myReadFrameSize);
    // Fail if we fail to read all of it.
    if (bytesRead < myReadFrameSize) {
        logWarnF("Truncated frame read: read %d bytes, expecting %d bytes", bytesRead, myReadFrameSize);
        return false;
        // How to resynchronise? Reset link?
    }
    return true;
}

bool ProtocolHandler::requestResponse() {
    BYTE8 tag = myTransactionBuffer[2];
    logDebugF("Read frame tag %02X (%s)", tag, tagToName(tag));
    resetWriteFrame();
    switch (tag) {
        case REQ_OPEN: {
            break;
        }
        case REQ_CLOSE: {
            break;
        }
        case REQ_READ: {
            break;
        }
        case REQ_WRITE: {
            break;
        }
        case REQ_GETS: {
            break;
        }
        case REQ_PUTS: {
            break;
        }
        case REQ_FLUSH: {
            break;
        }
        case REQ_SEEK: {
            break;
        }
        case REQ_TELL: {
            break;
        }
        case REQ_EOF: {
            break;
        }
        case REQ_FERROR: {
            break;
        }
        case REQ_REMOVE: {
            break;
        }
        case REQ_RENAME: {
            break;
        }
        case REQ_GETBLOCK: {
            break;
        }
        case REQ_PUTBLOCK: {
            break;
        }
        case REQ_ISATTY: {
            break;
        }
        case REQ_OPENREC: {
            break;
        }
        case REQ_GETREC: {
            break;
        }
        case REQ_PUTREC: {
            break;
        }
        case REQ_PUTEOF: {
            break;
        }
        case REQ_GETKEY: {
            break;
        }
        case REQ_POLLKEY: {
            break;
        }

        case REQ_GETENV: {
            break;
        }
        case REQ_TIME: {
            break;
        }
        case REQ_SYSTEM: {
            break;
        }
        case REQ_EXIT: {
            break;
        }

        case REQ_COMMAND: {
            break;
        }
        case REQ_CORE: {
            break;
        }
        case REQ_ID: {
            put((BYTE8) 0x00); // TODO extract real version from productVersion string
            break;
        }
        case REQ_GETINFO: {
            break;
        }

        case REQ_MSDOS: {
            break;
        }

        case REQ_FILEEXISTS: {
            break;
        }
        case REQ_TRANSLATE: {
            break;
        }
        case REQ_FERRSTAT: {
            break;
        }
        case REQ_COMMANDARG: {
            break;
        }

        default: {
            logWarnF("Frame tag %02X is unknown", tag);
            myUnimplementedFrameCount++;
            put(RES_UNIMPLEMENTED);
            break;
        }
    }
    return tag == REQ_EXIT;
}

void ProtocolHandler::resetWriteFrame() {
    myWriteFrameIndex = 2;
    // start putting data after the first two size fields
    // the size fields are set in fillInFrameSize()
}

bool ProtocolHandler::writeFrame() {
    if (myWriteFrameIndex == 0) {
        logWarn("No write frame has been prepared");
        return false;
    }
    WORD16 frameSize = fillInFrameSize();
    if (bDebug) {
        BYTE8 tag = myTransactionBuffer[2];
        logDebugF("Write frame: size word is %04X (%d) tag %02X (%s)",
                frameSize, frameSize, tag, tagToName(tag));
    }

    myIOLink.writeBytes(myTransactionBuffer, myWriteFrameIndex);
    return true;
}

// Wind back to the start to set the 2 length bytes.
WORD16 ProtocolHandler::fillInFrameSize() {
    // TODO inc by 1 if odd
    // TODO if incremented, set that final byte to 0
    if ((myWriteFrameIndex & (WORD16)0x01) == 0x01) {
        logWarn("WRITING ODD LENGTH FRAME - UNTESTED, NEEDS TO BE MADE EVEN BY PADDING 0");
    }

    WORD16 oldWriteFrameIndex = myWriteFrameIndex;
    myWriteFrameIndex = 0;
    WORD16 frameSize = (oldWriteFrameIndex - 2);
    put(frameSize);
    myWriteFrameIndex = oldWriteFrameIndex;
    return frameSize;
}

WORD64 ProtocolHandler::frameCount() {
    return myFrameCount;
}

WORD64 ProtocolHandler::badFrameCount() {
    return myBadFrameCount;
}

WORD64 ProtocolHandler::unimplementedFrameCount() {
    return myUnimplementedFrameCount;
}