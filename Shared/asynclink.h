//------------------------------------------------------------------------------
//
// File        : asynclink.h
// Description : Asynchronous link additions to Link.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 02/12/2025
//
// (C) 2005-2026 Matt J. Gumbley
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

    /**
     * Called under interrupt for accurate processing of hardware links, adjusting the
     * status word and LinkRegisters as needed.
     */
    virtual void clock() = 0;

    /**
     * What is the status of the link?
     * e.g. to determine whether a send has timed out, use statusWord & ST_DATA_SENT_NOT_ACKED.
     * This call does not change the status word, or the read/write registers of the
     * link, unlike writeComplete/readComplete.
     * @return The status word of the link, see ST_* bits defined in this header.
     */
    virtual WORD16 getStatusWord() = 0;

};

#endif // _ASYNCLINK_H