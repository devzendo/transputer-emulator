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

#include <functional>   // for the reschedule lambda

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

class Link {
public:
	Link(int linkNo, bool isServer);
	virtual void initialise(void) = 0;
	virtual ~Link(void);

	/**
	 * Enable extra debug diagnostics.
	 */
	void setDebug(bool newDebug);

	/**
	 * What is the number [0..4] of this link?
	 */
	int getLinkNo(void);

	/**
	 * Returns one of the LinkType_ constants above.
	 */
	virtual int getLinkType(void) = 0;

	/**
	 * Synchronous API for Link I/O. These calls may block. There are availability routines that can be called prior to
	 * calling these to determine whether they will block or not.
	 */
	virtual BYTE8 readByte(void) = 0;
	virtual void writeByte(BYTE8 b) = 0;

	/**
	 * Reset the link, and any state needed by it. Will interrupt any blocking read/write which has no way to signal
	 * to calling code that it has been interrupted.
	 * TODO Currently the link may not be in a usable state after this - this needs to change, so it's reusablu.
	 */
	virtual void resetLink(void) = 0;

	/**
	 * Further synchronous (blocking) calls that are implemented in terms of readByte/writeByte, for all link types.
	 */
	int readBytes(BYTE8* buffer, int bytesToRead);
	int writeBytes(BYTE8* buffer, int bytesToWrite);
	WORD16 readShort(void);
	void writeShort(WORD16 b);
	WORD32 readWord(void);
	void writeWord(WORD32 w);

	/**
	 * Availability - is there a byte available to read, and is the write buffer empty to accept a byte for writing?
	 * These routines are called by readComplete()/writeComplete(), for OperatingSystemLinks only.
	 */
	virtual bool readAvailable() = 0;
	virtual bool writeAvailable() = 0;

	/**
	 * Called periodically to effect transfer to/from the underlying Link to memory, if any transfers are active.
	 * If a transfer has ended (indicated by the completion routines readComplete()/writeComplete()), the callback is
	 * called with the Workspace pointer that was returned by the completion routines. (A typical callback from the
	 * emulator would reschedule the process with that Workspace).
	 */
	virtual void poll(std::function<void(WORD32)> reschedule);

    /**
     * Write a buffer through the link, asynchronously. When done, writeComplete() will
     * return the workspacePointer and clear down the transmit registers for the next write.
     * If the write is not complete, writeComplete will return NotProcess_p and the write
     * will continue.
     * @param workspacePointer The address in the Transputer's memory of a process that's
     * issuing an OUT/OUTBYTE/OUTWORD instruction. This process will be descheduled by
     * the emulator, and rescheduled when the transfer is complete. The link interface
     * doesn't care about this, but holds the workspace pointer for return by writeComplete.
     * poll() will pass that to its callback, causing a rescheduling.
     * @param dataPointer The address in real memory (physical, not Transputer-virtual)
     * of the start of the buffer to read data into.
     * @param length The length of the data to transfer.
     * @return Whether the send was initiated or not. If the link is busy, this can return false.
     */
    virtual bool writeDataAsync(WORD32 workspacePointer, BYTE8* dataPointer, WORD32 length) = 0;

    /**
     * Read data asynchronously into a sized buffer.
     * @param workspacePointer The address in the Transputer's memory of a process that's
     * issuing an IN instruction. This process will be descheduled by the emulator, and
     * rescheduled when the transfer is complete. The link interface doesn't care about
     * this, but holds the workspace pointer for return by readComplete.
     * poll() will pass that to its callback, causing a rescheduling.
     * @param dataPointer The address in real memory (physical, not Transputer-virtual)
     * of the start of the buffer to read data into.
     * @param length The amount of data to read into the buffer at dataPointer.
     */
    virtual void readDataAsync(WORD32 workspacePointer, BYTE8* dataPointer, WORD32 length) = 0;

	/**
	 * The completion routines readComplete()/writeComplete() will perform I/O if there is data, and adjust the
	 * pointer/length in the LinkRegister. If the transfer is complete, the LinkRegister's Workspace pointer will be
	 * returned, and the LinkRegister cleared down. If the transfer is not complete, NotProcess.p will be returned.
	 */

	/**
	 * Has the write completed? Called by poll() and blocking writeByte().
	 * @return NotProcess_p if the write has not yet completed, or the address in the
	 * Transputer's memory of a process to reschedule (and in this case, the write
	 * registers are reset for the next write). Note that if you receive a workspace
	 * pointer on one call, you'll get NotProcess_p on the next - you MUST use the
	 * pointer when you get it.
	 */
	virtual WORD32 writeComplete() = 0;

    /**
     * Has the read completed? Called by poll() and blocking readByte().
     * @return NotProcess_p if the read has not yet completed, or the address in the
     * Transputer's memory of a process to reschedule (and in this case, the read
     * registers are reset for the next read). Note that if you receive a workspace
     * pointer on one call, you'll get NotProcess_p on the next - you MUST use the
     * pointer when you get it.
     */
    virtual WORD32 readComplete() = 0;

protected:
	int myLinkNo;
	bool bServer;
	bool bDebug;
    WORD32 myWriteSequence, myReadSequence;
	LinkRegisters m_send_registers{};
	LinkRegisters m_receive_registers{};
};

#endif // _LINK_H

