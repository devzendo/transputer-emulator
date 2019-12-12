//------------------------------------------------------------------------------
//
// File        : link.h
// Description : Abstract base class for links
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 18/07/2005
//
// (C) 2005-2019 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _LINK_H
#define _LINK_H

#include <exception>

#include "platformdetection.h"
#include "types.h"

#ifdef PLATFORM_WINDOWS
// For MSVC, shut up about throw (exception)
#pragma warning( disable : 4290 )
#endif

const int LinkType_FIFO = 0;
const int LinkType_Socket = 1;
const int LinkType_SharedMemory = 2;
const int LinkType_NamedPipe = 3;
const int LinkType_Stub = 4; // for testing

class Link {
public:
	Link(int linkNo, bool isServer);
	virtual void initialise(void) throw (std::exception) = 0;
	virtual ~Link(void);
	// TODO may need buffer-centric versions of these, and
	// these are entirely synchronous. to prevent blocking I/O
	// suspending the whole emulator, I should make each link
	// driven by a state machine to introduce some polled
	// asynchrony.
	// For now, just to get data flowing from the IServer,
	// I'll use the synchronous forms.
	virtual BYTE8 readByte(void) throw (std::exception) = 0;
	virtual void writeByte(BYTE8 b) throw (std::exception) = 0;
	int readBytes(BYTE8* buffer, int bytesToRead) throw (std::exception);
	int writeBytes(BYTE8* buffer, int bytesToWrite) throw (std::exception);
	WORD16 readShort(void) throw (std::exception);
    void writeShort(WORD16 b) throw (std::exception);
	WORD32 readWord(void) throw (std::exception);
	void writeWord(WORD32 w) throw (std::exception);
	virtual void resetLink(void) throw (std::exception) = 0;
	int getLinkNo(void);
	void setDebug(bool newDebug);
	virtual int getLinkType(void) = 0;
protected:
	int myLinkNo;
	bool bServer;
	bool bDebug;
};

#endif // _LINK_H

