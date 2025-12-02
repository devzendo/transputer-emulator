//------------------------------------------------------------------------------
//
// File        : asynclink.h
// Description : Asynchronous link additions to Link.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 02/12/2025
//
// (C) 2005-2025 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _ASYNCLINK_H
#define _ASYNCLINK_H

#include "types.h"

class AsyncLink {
public:
    virtual ~AsyncLink() = default;

    virtual void clock() = 0;

    /**
     * Write a buffer through the link, asynchronously. When done, writeComplete() will
     * return the workspacePointer and clear dowh the transmit registers for the next write.
     * If the write is not complete, writeComplete will return NotProcess_p and the write
     * will continue.
     * @param workspacePointer The address in the Transputer's memory of a process that's
     * issuing an OUT/OUTBYTE/OUTWORD instruction. This process will be descheduled by
     * the emulator, and rescheduled when the transfer is complete. The link interface
     * doesn't care about this, but holds the workspace pointer for return by readComplete.
     * @param dataPointer
     * @param length
     * @return
     */
    virtual bool writeDataAsync(WORD32 workspacePointer, BYTE8* dataPointer, WORD32 length) = 0;

    /**
     * Has the write completed?
     * @return NotProcess_p if the write has not yet completed, or the address in the
     * Transputer's memory of a process to reschedule (and in this case, the write
     * registers are reset for the next write). Note that if you receive a workspace
     * pointer on one call, you'll get NotProcess_p on the next - you must use the
     * pointer when you get it.
     */
    virtual WORD32 writeComplete() = 0;

    /**
     * Read data asynchronously into a sized buffer.
     * @param workspacePointer The address in the Transputer's memory of a process that's
     * issuing an IN instruction. This process will be descheduled by the emulator, and
     * rescheduled when the transfer is complete. The link interface doesn't care about
     * this, but holds the workspace pointer for return by readComplete.
     * @param dataPointer The address in real memory (physical, not Transputer-virtual)
     * of the start of the buffer to read data into.
     * @param length The amount of data to read into the buffer at dataPointer.
     */
    virtual void readDataAsync(WORD32 workspacePointer, BYTE8* dataPointer, WORD32 length) = 0;

    /**
     * Has the read completed?
     * @return NotProcess_p if the read has not yet completed, or the address in the
     * Transputer's memory of a process to reschedule (and in this case, the read
     * registers are reset for the next read). Note that if you receive a workspace
     * pointer on one call, you'll get NotProcess_p on the next - you must use the
     * pointer when you get it.
     */
    virtual WORD32 readComplete() = 0;
};

#endif // _ASYNCLINK_H