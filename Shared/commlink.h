//------------------------------------------------------------------------------
//
// File        : commlink.h
// Description : Implementation of a link that uses a Windows COM port.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 17/06/2026
//
// (C) 2005-2026 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef COMMLINK_H
#define COMMLINK_H

#include "types.h"
#include "link.h"

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h> // For HANDLE
#include <fileapi.h> // for I/O functions.

class CommLink : public Link {
public:
    CommLink(int linkNo, bool isServer, const std::string &comPortName);
    void initialise() override;
    ~CommLink() override;
    BYTE8 readByte() override;
    void writeByte(BYTE8 b) override;
    void resetLink() override;
    int getLinkType() override;
private:
    static constexpr int COM_MSGBUF_SIZE = 128;
    HANDLE myHandle;
    WORD32 myWriteSequence, myReadSequence;
    std::string myComName;
	char myMsgbuf[COM_MSGBUF_SIZE]{};
};

#endif // COMMLINK_H
