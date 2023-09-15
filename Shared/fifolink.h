//------------------------------------------------------------------------------
//
// File        : fifolink.h
// Description : Implementation of link that uses a FIFO.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 19/07/2005
//
// (C) 2005-2023 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _FIFOLINK_H
#define _FIFOLINK_H

#include <exception>

#include "types.h"
#include "link.h"

class FIFOLink : public Link {
public:
	FIFOLink(int linkNo, bool isServer);
	void initialise(void) throw (std::exception);
	~FIFOLink(void);
	BYTE8 readByte(void) throw (std::exception);
	void writeByte(BYTE8 b) throw (std::exception);
	void resetLink(void) throw (std::exception);
	int getLinkType(void);
private:
	int myWriteFD, myReadFD;
	WORD32 myWriteSequence, myReadSequence;
	char myReadFifoName[80];
	char myWriteFifoName[80];
};

#endif // _FIFOLINK_H

