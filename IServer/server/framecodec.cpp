//------------------------------------------------------------------------------
//
// File        : framecodec.cpp
// Description : Encoding/Decoding of elements in a protocol frame buffer
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 15/11/2019
//
// (C) 2005-2019 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include "framecodec.h"
#include "log.h"


void FrameCodec::put(const BYTE8 byte8) {
    logDebugF("put BYTE8 %02X @ %04X", byte8, myWriteFrameIndex);
    myTransactionBuffer[myWriteFrameIndex++] = byte8;
}

void FrameCodec::put(const WORD16 word16) {
    logDebugF("put WORD16 %04X @ %04X UNFINISHED TEST", word16, myWriteFrameIndex);
    myTransactionBuffer[myWriteFrameIndex++] = (BYTE8) (word16 & (BYTE8)0xff);

// TODO how to test this?    myTransactionBuffer[myWriteFrameIndex++] = (BYTE8) (word16 >> (BYTE8)8) & (BYTE8)0xff;
    myTransactionBuffer[myWriteFrameIndex++] = (BYTE8) 0;  // BAD DATA - NEED TEST TO ESTABLISH ABOVE
}

void FrameCodec::put(const WORD32 word32) {
    logDebugF("put WORD32 %08X @ %04X UNIMPLEMENTD", word32, myWriteFrameIndex);

}

BYTE8 FrameCodec::get8() {
    return myTransactionBuffer[myReadFrameIndex++];
}

WORD16 FrameCodec::get16() {
    // Input a little-endian short, LSB first MSB last
    return (get8()) |
           (get8() << 8);
}

WORD32 FrameCodec::get32() {
    // Input a little-endian word, LSB first MSB last
    return (get8()) |
           (get8() << 8) |
           (get8() << 16) |
           (get8() << 24);
}

// A max length protocol frame containing just a string (imagine max protocol frame size is 8):
// FL FM SL SM S0 S1 S2 S3
// ^     ^     ^
// Frame |      \
// Size  |       String bytes, 4 in all, not null terminated
// (6,0) String
//       Size (4,0)
// String max length is TransactionBufferSize - 2 - 2
// (- frame size bytes - string size bytes)
std::string FrameCodec::getString() {
    WORD16 stringLen = get16();

    return std::string("");
}

void FrameCodec::resetWriteFrame() {
    myWriteFrameIndex = 2;
    // start putting data after the first two size fields
    // the size fields are set in fillInFrameSize()
}

// Wind back to the start to set the 2 length bytes.
WORD16 FrameCodec::fillInFrameSize() {
    if ((myWriteFrameIndex & (WORD16)0x01) == 0x01) {
        logDebug("Padding odd length frame with 00");
        put((BYTE8) 0x00); // increments myWriteFrameIndex
    }

    WORD16 oldWriteFrameIndex = myWriteFrameIndex;
    myWriteFrameIndex = 0;
    WORD16 frameSize = (oldWriteFrameIndex - 2);
    put(frameSize);
    myWriteFrameIndex = oldWriteFrameIndex;
    return frameSize;
}
