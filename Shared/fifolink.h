//------------------------------------------------------------------------------
//
// File        : fifolink.h
// Description : Implementation of link that uses a FIFO.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 19/07/2005
//
// (C) 2005-2019 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _FIFOLINK_H
#define _FIFOLINK_H

#include <exception>
using namespace std;

#include "types.h"
#include "link.h"

class FIFOLink : public Link {
public:
	FIFOLink(int linkNo, bool isServer);
	void initialise(void) throw (exception);
	~FIFOLink(void);
	BYTE readByte(void) throw (exception);
	void writeByte(BYTE b) throw (exception);
	void resetLink(void) throw (exception);
private:
	int myWriteFD, myReadFD;
	WORD32 myWriteSequence, myReadSequence;
};

#endif // _FIFOLINK_H

