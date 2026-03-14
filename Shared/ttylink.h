//------------------------------------------------------------------------------
//
// File        : ttylink.h
// Description : Implementation of a link that uses a TTY or COM port.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 14/03/2026
//
// (C) 2005-2026 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef TTYLINK_H
#define TTYLINK_H

#include "types.h"
#include "link.h"

class TTYLink : public Link {
public:
    TTYLink(int linkNo, bool isServer, const std::string &ttyFileName);
    void initialise() override;
    ~TTYLink() override;
    BYTE8 readByte() override;
    void writeByte(BYTE8 b) override;
    void resetLink() override;
    int getLinkType() override;
private:
    static constexpr int TTY_MSGBUF_SIZE = 128;
    int myFD;
    WORD32 myWriteSequence, myReadSequence;
    std::string myTTYName;
	char myMsgbuf[TTY_MSGBUF_SIZE]{};
};

#endif // TTYLINK_H