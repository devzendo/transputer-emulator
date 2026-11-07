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
    // what if there's already data available?
}

WORD32 SystemLink::writeComplete() {
    return NotProcess_p; // TODO

    // if writeLinkRegister.workspace == NotProcess.p || !writeAvailable() // optimise?
    //   return NotProcess.p
    // writeByte(*writeLinkRegister.buffer++)
    // if --writeLinkRegister.length == 0
    //   // Flush underlying mechanism
    //   WORD32 t = writeLinkRegister.workspace
    //   writeLinkRegister.workspace = NotProcess.p
    //   writeLinkRegister.buffer = nullptr
    //   return t; // it'll get rescheduled
    // return NotProcess.p
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
