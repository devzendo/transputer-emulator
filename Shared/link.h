//------------------------------------------------------------------------------
//
// File        : link.h
// Description : Abstract base class for links
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 18/07/2005
//
// (C) 2005-2026 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _LINK_H
#define _LINK_H

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
const int LinkType_TVS = 5;
const int LinkType_Null = 6;
const int LinkType_Async = 7;
const int LinkType_USBCDC = 8;
const int LinkType_TTY = 9;
const int LinkType_InMemory = 10;

class Link {
public:
	Link(int linkNo, bool isServer);
	virtual void initialise(void) = 0;
	virtual ~Link(void);

	// Synchronous API for Link I/O. These calls may block. There are availability routines that can be called prior to
	// calling these to determine whether they will block or not.
	virtual BYTE8 readByte(void) = 0;
	virtual void writeByte(BYTE8 b) = 0;

	// Reset the link, and any state needed by it.
	virtual void resetLink(void) = 0;

	// Returns one of the LinkType_ constants above.
	virtual int getLinkType(void) = 0;

	// Further synchronous calls that are implemented in terms of readByte/writeByte.
	int readBytes(BYTE8* buffer, int bytesToRead);
	int writeBytes(BYTE8* buffer, int bytesToWrite);
	WORD16 readShort(void);
	void writeShort(WORD16 b);
	WORD32 readWord(void);
	void writeWord(WORD32 w);

	// What is the number [0..4] of this link?
	int getLinkNo(void);

	// Enable extra debug diagnostics.
	void setDebug(bool newDebug);

	// Availability - is there a byte available to read, and is the write buffer empty to accept a byte for writing?
	virtual bool readAvailable() = 0;
	virtual bool writeAvailable() = 0;

protected:
	int myLinkNo;
	bool bServer;
	bool bDebug;
    WORD32 myWriteSequence, myReadSequence;
};

/* Used for the read and write end of an asynchronous link; LinkRegisters are used by the link during transfers between
 * physical memory and by the CPU when a Link has finished performing requested I/O to record the workspace pointer of
 * the process that be rescheduled when the transfer has completed.
 */
struct LinkRegisters {
public:
	WORD32 m_workspace_pointer;
	BYTE8* m_data_pointer;
	WORD32 m_length;
};


#endif // _LINK_H

