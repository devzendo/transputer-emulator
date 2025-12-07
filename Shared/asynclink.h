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

/*
 * Status word bits.
 * 15       | 14       | 13        | 12        | 11        | 10        | 9         | 8
 * FRAMING  | OVERRUN  | READ DATA | READY TO  | DATA SENT | READ      | SEND      | ......... |
 *          |          | AVAILABLE | SEND      | NOT ACKED | COMPLETE  | COMPLETE  |           |
 *          |          |           |           | (TIMEOUT) |           |           |           |
 * ---------------------------------------------------------------------------------------------
 * 7        | 6        | 5         | 4         | 3         | 2         | 1         | 0
 * DATA RECEIVED IF READ DATA AVAILABLE (BIT 13) IS TRUE
 */
const WORD16 ST_FRAMING = 0x8000;
const WORD16 ST_OVERRUN = 0x4000;
const WORD16 ST_READ_DATA_AVAILABLE = 0x2000;
const WORD16 ST_READY_TO_SEND = 0x1000;
const WORD16 ST_DATA_SENT_NOT_ACKED = 0x0800;
const WORD16 ST_READ_COMPLETE = 0x0400;
const WORD16 ST_SEND_COMPLETE = 0x0200;
const WORD16 ST_DATA_MASK = 0x00FF;

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
     * What is the status of the link?
     * e.g. to determine whether a send has timed out, use statusWord & ST_DATA_SENT_NOT_ACKED.
     * This call does not change the status word, or the read/write registers of the
     * link, unlike writeComplete/readComplete.
     * @return The status word of the link, see ST_* bits defined in this header.
     */
    virtual WORD16 getStatusWord() = 0;

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