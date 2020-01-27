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
// (C) 2005-2020 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <string>
#include "types.h"
#include "constants.h"
#include "platform.h"
#include "link.h"
#include "hexdump.h"
#include "log.h"
#include "protocolhandler.h"
#include "platformdetection.h"
#include "../isproto.h"

namespace {
    const char* tagToName(const BYTE8 tag) {
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

ProtocolHandler::~ProtocolHandler() = default;

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
    // Frame length encoded in the first two bytes (little endian).
    // Minimum frame length is 8 (ie minimum message length of 6 bytes in the to-server
    // direction), maximum 512. Packet size must always be an even number of bytes.
    // (TDS 2nd ed, p356, sec 16.5.1.)
    // Inmos iServer has max packet size of 1040 ? Is that from T9000?

    // Read 8 bytes into transaction buffer; extract frame size from first two bytes.
    // Fail if we fail to read 8 bytes.
    codec.setReadFrameSize(myIOLink.readShort());
    // TODO when link abstraction is improved to add timeout reads, and analyse/reset/error
    // rework this to take those into account.
    myFrameCount++;
    if (bDebug) {
        logDebugF("Read frame size word is %04X (%d)", codec.getReadFrameSize(), codec.getReadFrameSize());
    }
    if (codec.readFrameSizeOutOfRange()) {
        logWarnF("Read frame size %04X out of range", codec.getReadFrameSize());
        myBadFrameCount++;
        return false;
    }
    if ((codec.getReadFrameSize() & 0x01) == 0x01) {
        logWarnF("Read frame size %04X is odd", codec.getReadFrameSize());
        myBadFrameCount++;
        return false;
    }
    // Fail if frame size > max frame length.
    // Store frameSize at positions 0 and 1 in myTransactionBuffer
    codec.fillInReadFrameSize();
    // Read remaining data.
    WORD16 bytesRead = myIOLink.readBytes(&codec.myTransactionBuffer[2], codec.getReadFrameSize());
    // Fail if we fail to read all of it.
    if (bytesRead < codec.getReadFrameSize()) {
        logWarnF("Truncated frame read: read %d bytes, expecting %d bytes", bytesRead, codec.getReadFrameSize());
        return false;
        // How to resynchronise? Reset link?
    }
    if (bDebug) {
        auto len = codec.getReadFrameSize() + 2;
        hexdump(codec.myTransactionBuffer, len);
    }
    return true;
}

bool ProtocolHandler::requestResponse() {
    BYTE8 tag = codec.myTransactionBuffer[2];
    logDebugF("Read frame tag %02X (%s)", tag, tagToName(tag));
    codec.resetWriteFrame();
    codec.myReadFrameIndex = 3;
    switch (tag) {
        case REQ_OPEN: {
            reqOpen();
            break;
        }
        case REQ_CLOSE: {
            break;
        }
        case REQ_READ: {
            reqRead();
            break;
        }
        case REQ_WRITE: {
            reqWrite();
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
            reqExit();
            break;
        }

        case REQ_COMMAND: {
            break;
        }
        case REQ_CORE: {
            break;
        }
        case REQ_ID: {
            reqId();
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
            codec.put(RES_UNIMPLEMENTED);
            break;
        }
    }
    return tag == REQ_EXIT;
}

bool ProtocolHandler::writeFrame() {
    if (codec.myWriteFrameIndex == 0) {
        logWarn("No write frame has been prepared");
        return false;
    }
    WORD16 frameSize = codec.fillInFrameSize();
    if (bDebug) {
        BYTE8 tag = codec.myTransactionBuffer[2];
        logDebugF("Write frame: size word is %04X (%d) tag %02X (%s)",
                frameSize, frameSize, tag, tagToName(tag));
        hexdump(codec.myTransactionBuffer, codec.myWriteFrameIndex);
    }
    myIOLink.writeBytes(codec.myTransactionBuffer, codec.myWriteFrameIndex);
    return true;
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

int ProtocolHandler::exitCode() {
    return myExitCode;
}

// Protocol frame handlers ---------------------------------------------------------------------------------------------

void ProtocolHandler::reqOpen() {
    const std::string filename = codec.getString();
    const BYTE8 openType = codec.get8();
    const BYTE8 openMode = codec.get8();
    // TODO to be continued...
}

void ProtocolHandler::reqRead() {
    const WORD32 streamId = codec.get32();
    const WORD16 size = codec.get16();
    // TODO if length < 1 nothing happens....
    try {
        // TODO last op must not have been a write
        logDebugF("Reading %d bytes from stream #%d", size, streamId);
        // Read directly into the transaction buffer, but don't know whether this'll succeed, nor how much will
        // actually be read, so can't fill those in yet... don't want to copy data around...
        // Note that use of writeOffset does NOT change the writeFrameIndex.
        BYTE8 *dataBuffer = (BYTE8 *) codec.writeOffset(5);
        WORD16 read = myPlatform.readStream(streamId, size, dataBuffer);
        // Offsets into the transaction buffer...
        // 0 frame size lsb
        // 1 frame size msb
        // 2 RES_SUCCESS
        // 3 read size lsb
        // 4 read size msb
        // 5 data... where the above call to readStream has stored it without moving the index from offset 2
        logDebugF("Read %d bytes from stream #%d", read, streamId);
        codec.put(RES_SUCCESS); // direct read into writeOffset(5) above didn't change writeFrameIndex
        codec.put((WORD16) read);
        // The codec needs to know how much data was read into writeOffset(5) above so it can fill in the frame size.
        codec.advance(read);
    } catch (const std::range_error &e) {
        logWarn(e.what());
        codec.put(RES_BADID);
        codec.put((WORD16) 0);
    } catch (const std::invalid_argument &e) {
        logWarn(e.what());
        codec.put(RES_BADID);
        codec.put((WORD16) 0);
    }
}

void ProtocolHandler::reqWrite() {
    const WORD32 streamId = codec.get32();
    const std::string data = codec.getString(); // can be binary, this is fine.. Size limited to WORD16.
    try {
        // TODO last op must not have been a read
        WORD16 size = data.size();
        logDebugF("Writing %d bytes to stream #%d", size, streamId);
        WORD16 wrote = (size > 0) ? myPlatform.writeStream(streamId, size, (BYTE8 *) data.data()) : 0;
        logDebugF("Wrote %d bytes to stream #%d", wrote, streamId);
        // TODO if streamId == 1 or 2, flush (test after open done, so we can correctly sense presence/absence of flush call on platform
        codec.put(RES_SUCCESS);
        codec.put((WORD16) wrote);
    } catch (const std::range_error &e) {
        logWarn(e.what());
        codec.put(RES_BADID);
        codec.put((WORD16) 0);
    } catch (const std::invalid_argument &e) {
        logWarn(e.what());
        codec.put(RES_BADID);
        codec.put((WORD16) 0);
    }
}

void ProtocolHandler::reqExit() {
    const WORD32 status = codec.get32();
    logDebugF("Exit status received as %08X", status);

    switch (status) {
        case RES_EXIT_SUCCESS:
            myExitCode = 0;
            break;
        case RES_EXIT_FAILURE:
            myExitCode = 1;
            break;
        default:
            myExitCode = (int)status;
    }
    logDebugF("Exit code set to %04X", myExitCode);
    codec.put(RES_SUCCESS);
}

void ProtocolHandler::reqId() {
    codec.put(RES_SUCCESS);
    codec.put((BYTE8) 0x00); // Version: TODO extract real version from productVersion string
#if defined(PLATFORM_WINDOWS)
    codec.put((BYTE8) 0X01); // Host: "PC"
    codec.put((BYTE8) 0X06); // OS: "Windows" (addition: not in iServer docs)
#elif defined(PLATFORM_OSX)
    codec.put((BYTE8) 0X09); // Host: "Mac" (addition: not in iServer docs)
    codec.put((BYTE8) 0X07); // OS: "macOS" (addition: not in iServer docs)
#elif defined(PLATFORM_LINUX)
    codec.put((BYTE8) 0X01); // Host: "PC"
    codec.put((BYTE8) 0X08); // OS: "Linux" (addition: not in iServer docs)
#else
    codec.put((BYTE8) 0X00); // Host: ?
    codec.put((BYTE8) 0X00); // OS: ?
#endif
    codec.put((BYTE8) ((BYTE8)myIOLink.getLinkType() & (BYTE8)0xff)); // Board: actually the link type
}