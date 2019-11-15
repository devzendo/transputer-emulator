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

const int TransactionBufferSize = 1040;
const int StringBufferSize = TransactionBufferSize - 2 - 2 + 1; // - frame size bytes - string length bytes + \0


class FrameCodec {
public:
    FrameCodec():
            myReadFrameSize(0), myReadFrameIndex(0), myWriteFrameIndex(0) {};

    void put(const BYTE8 byte8);
    void put(const WORD16 word16);
    void put(const WORD32 word32);
    BYTE8 get8();
    WORD16 get16();
    WORD32 get32();
    std::string getString();
    
    WORD16 getReadFrameSize();
    void setReadFrameSize(const WORD16 size);
    bool readFrameSizeOutOfRange();

    void resetWriteFrame();
    WORD16 fillInFrameSize();

    BYTE8 myTransactionBuffer[TransactionBufferSize];
    BYTE8 myStringBuffer[StringBufferSize]; // TODO keep this?
    WORD16 myReadFrameIndex;
    WORD16 myWriteFrameIndex;
private:
    WORD16 myReadFrameSize; // set if readFrame returns true
};

#endif //_FRAMECODEC_H
