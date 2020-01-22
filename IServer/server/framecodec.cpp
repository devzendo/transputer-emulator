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

#include <cstring>
#include <stdexcept>
#include "framecodec.h"
#include "log.h"

FrameCodec::FrameCodec(): myReadFrameSize(0), myReadFrameIndex(0), myWriteFrameIndex(0) {
    std::memset(myTransactionBuffer, 0, sizeof(myTransactionBuffer));
    std::memset(myStringBuffer, 0, sizeof(myStringBuffer));
}

WORD16 FrameCodec::getReadFrameSize() {
    return myReadFrameSize;
}

void FrameCodec::setReadFrameSize(const WORD16 size) {
    myReadFrameSize = size;
}

bool FrameCodec::readFrameSizeOutOfRange() {
    return myReadFrameSize < 6 || myReadFrameSize > 510;
}

void FrameCodec::put(const BYTE8 byte8) {
    logDebugF("put @ %04X BYTE8  %02X", myWriteFrameIndex, byte8);
    myTransactionBuffer[myWriteFrameIndex++] = byte8;
}

void FrameCodec::put(const WORD16 word16) {
    logDebugF("put @ %04X WORD16 %04X", myWriteFrameIndex, word16);
    myTransactionBuffer[myWriteFrameIndex++] = (BYTE8) (word16 & (BYTE8)0xff);
    myTransactionBuffer[myWriteFrameIndex++] = (BYTE8) (word16 >> (BYTE8)8) & (BYTE8)0xff;
}

void FrameCodec::put(const WORD32 word32) {
    logDebugF("put @ %04X WORD32 %08X", myWriteFrameIndex, word32);
    myTransactionBuffer[myWriteFrameIndex++] = (BYTE8) (word32 & (BYTE8)0xff);
    myTransactionBuffer[myWriteFrameIndex++] = (BYTE8) (word32 >> (BYTE8)8) & (BYTE8)0xff;
    myTransactionBuffer[myWriteFrameIndex++] = (BYTE8) (word32 >> (BYTE8)16) & (BYTE8)0xff;
    myTransactionBuffer[myWriteFrameIndex++] = (BYTE8) (word32 >> (BYTE8)24) & (BYTE8)0xff;
}

BYTE8 FrameCodec::get8() {
    return myTransactionBuffer[myReadFrameIndex++];
}

WORD16 FrameCodec::get16() {
    // Input a little-endian short, LSB first MSB last
    return  get8() |
           (get8() << 8);
}

WORD32 FrameCodec::get32() {
    // Input a little-endian word, LSB first MSB last
    return  get8() |
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
std::string FrameCodec::getString() noexcept(false) {
    WORD16 stringLen = get16();
    if (stringLen > StringBufferSize) {
        logWarnF("String in frame is %d bytes - exceeding maximum of %d", stringLen, StringBufferSize);
        throw std::range_error("String in frame exceeds maximum size");
    }
    const auto *buf = const_cast<const BYTE8 *>(myTransactionBuffer + myReadFrameIndex);
    const char *cbuf = (const char *)buf;
    myReadFrameIndex += stringLen;
    return std::string(cbuf, (int)stringLen);
}

void FrameCodec::resetWriteFrame() {
    myWriteFrameIndex = 2;
    // start putting data after the first two size fields
    // the size fields are set in fillInFrameSize()
}

// Wind back to the start to set the 2 length bytes.
void FrameCodec::fillInReadFrameSize() {
    myTransactionBuffer[0] = (BYTE8) (getReadFrameSize() & (BYTE8)0xff);
    myTransactionBuffer[1] = (BYTE8) (getReadFrameSize() >> (BYTE8)8) & (BYTE8)0xff;
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

// Obtain the address of a particular offset into the transaction buffer.
BYTE8 *FrameCodec::writeOffset(const WORD16 offset) {
    return myTransactionBuffer + offset;
}

// Increase the write frame index by a number of bytes.
void FrameCodec::advance(const WORD16 amount) {
    myWriteFrameIndex += amount;
}
