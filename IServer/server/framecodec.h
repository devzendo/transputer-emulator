//------------------------------------------------------------------------------
//
// File        : framecodec.h
// Description : Encoding/Decoding of elements in a protocol frame buffer
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 15/11/2019
//
// (C) 2005-2019 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _FRAMECODEC_H
#define _FRAMECODEC_H

#include <string>
#include "types.h"

const int TransactionBufferSize = 512; // From TDS 2nd ed, p356, sec 16.5.1.
const int StringBufferSize = TransactionBufferSize - 2 - 2; // - frame size bytes - string length bytes


class FrameCodec {
public:
    FrameCodec();

    void put(BYTE8 byte8);
    void put(WORD16 word16);
    void put(WORD32 word32);
    BYTE8 get8();
    WORD16 get16();
    WORD32 get32();
    std::string getString() noexcept(false);

    WORD16 getReadFrameSize();
    void setReadFrameSize(WORD16 size);
    bool readFrameSizeOutOfRange();

    void resetWriteFrame();
    void fillInReadFrameSize();
    WORD16 fillInFrameSize();
    BYTE8 *writeOffset(WORD16 offset);
    void advance(WORD16 amount);

    BYTE8 myTransactionBuffer[TransactionBufferSize];
    BYTE8 myStringBuffer[StringBufferSize]; // TODO keep this?
    WORD16 myReadFrameIndex;
    WORD16 myWriteFrameIndex;
private:
    WORD16 myReadFrameSize; // set if readFrame returns true
};

#endif //_FRAMECODEC_H
