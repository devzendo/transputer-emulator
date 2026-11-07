//------------------------------------------------------------------------------
//
// File        : systemlink.cpp
// Description : Implementation of Operating System Links (as opposed to Interrupt-
//               Driven Links: see asynclink.h).
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 16/10/2026
//
// (C) 2005-2026 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include "constants.h"
#include "log.h"
#include "systemlink.h"

SystemLink::SystemLink(int linkNo, bool isServer) : Link(linkNo, isServer) {
    logDebugF("Constructing system link %d for %s", myLinkNo, isServer ? "server" : "cpu client");
}

SystemLink::~SystemLink() {
    logDebugF("Destroying system link %d", myLinkNo);
}

bool SystemLink::writeDataAsync(WORD32 workspacePointer, BYTE8* dataPointer, WORD32 length) {
    if (writeAvailable()) {
        logDebugF("Link %d sending %d bytes from initial address 0x%08x for process 0x%08x", myLinkNo, length, dataPointer, workspacePointer);
        m_send_registers.m_workspace_pointer = workspacePointer;
        m_send_registers.m_data_pointer = dataPointer;
        m_send_registers.m_length = length;
        return true;
    }
    logWarnF("Link %d write not available - not sending %d bytes from initial address 0x%08x for process 0x%08x", myLinkNo, length, dataPointer, workspacePointer);
    return false;
}

void SystemLink::readDataAsync(WORD32 workspacePointer, BYTE8* dataPointer, WORD32 length) {
    m_receive_registers.m_workspace_pointer = workspacePointer;
    m_receive_registers.m_data_pointer = dataPointer;
    m_receive_registers.m_length = length;
    // TODO what if there's already data available?
}

WORD32 SystemLink::writeComplete() {
    if (m_send_registers.m_workspace_pointer == NotProcess_p || !writeAvailable()) {
        // TODO optimise for multiple bytes? loop?
        return NotProcess_p;
    }
    logDebugF("Link %d sending next data", myLinkNo);
    writeByte(*m_send_registers.m_data_pointer++);
    if (--m_send_registers.m_length == 0) {
        logDebugF("Link %d is now complete", myLinkNo);
        // SystemLinks have no acknowledgement mechanism, as GPIOAsyncLinks do. Flushing is as good as it gets, in the
        // absence of some other mechanism - some encapsulation mechanism perhaps, so that when we get <end of frame>
        // we assume the remote end has acknowledged?
        // The occam 2 Reference Manual (INMOS 72 occ 45 01, Prentice Hall, 1988), §1.2, says each channel provides
        // unbuffered, unidirectional point-to-point communication between two concurrent processes. In §1.2.2 it says
        // "The output waits until the value has been received by a corresponding input."
        // INMOS Technical Note 18, Connecting INMOS links (72-TCH-018), says a link is the hardware implementation of
        // an occam channel. Concerning the final byte: the ack signifies that a process was able to receive the byte
        // and that the receiving link can accept another, and the sending link reschedules the sending process only
        // after the ack for the final byte has been received.
        // TODO Flush underlying mechanism when there's a call to effect this.
        const WORD32 t = m_send_registers.m_workspace_pointer;
        m_send_registers.m_workspace_pointer = NotProcess_p;
        m_send_registers.m_data_pointer = nullptr;
        m_send_registers.m_length = 0;
        return t;
        // This process will get rescheduled by the CPU - but that's not the concern of this link engine, it just
        // informs the CPU which process is now due to be rescheduled.
    }
    return NotProcess_p;
}

WORD32 SystemLink::readComplete() {
    return NotProcess_p; // TODO
    // if readLinkRegister.workspace == NotProcess.p || !readAvailable() // could be optimised to return the number of bytes available
    //   return NotProcess.p
    // *readLinkRegister.buffer++ = readByte(); // similarly, block read/memory write?
    // if --readLinkRegister.length == 0 // similarly, subtract amount read
    //   WORD32 t = readLinkRegister.workspace
    //   readLinkRegister.workspace = NotProcess.p
    //   readLinkRegister.buffer = nullptr
    //   return t // it'll get rescheduled
    // return NotProcess.p

}
