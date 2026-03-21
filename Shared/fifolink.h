//------------------------------------------------------------------------------
//
// File        : fifolink.h
// Description : Implementation of link that uses a FIFO.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 19/07/2005
//
// (C) 2005-2026 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef FIFOLINK_H
#define FIFOLINK_H

#include "types.h"
#include "link.h"

class FIFOLink : public Link {
public:
	FIFOLink(int linkNo, bool isServer);
	void initialise() override;
	~FIFOLink() override;
	BYTE8 readByte() override;
	void writeByte(BYTE8 b) override;
	void resetLink() override;
	int getLinkType() override;
private:
    static constexpr int FIFO_MSGBUF_SIZE = 128;
	int myWriteFD, myReadFD;
	WORD32 myWriteSequence, myReadSequence;
	char myReadFifoName[80]{};
	char myWriteFifoName[80]{};
	char myMsgbuf[FIFO_MSGBUF_SIZE]{};
};

#endif // FIFOLINK_H

