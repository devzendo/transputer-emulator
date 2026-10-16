//------------------------------------------------------------------------------
//
// File        : link.cpp
// Description : Abstract base class for links
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 18/07/2005
//
// (C) 2005-2026 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include "types.h"
#include "constants.h"
#include "link.h"
#include "log.h"

Link::Link(int linkNo, bool isServer) {
	myLinkNo = linkNo;
	bServer = isServer;
	bDebug = false;
    myWriteSequence = myReadSequence = 0;
	logDebugF("Constructing %s link %d", bServer ? "server" : "CPU client", myLinkNo);
}

Link::~Link(void) {
	logDebugF("Destroying link %d", myLinkNo);
}

int Link::readBytes(BYTE8* buffer, int bytesToRead) {
    // TODO improve inefficiency
    int readCount = 0;
    for (; readCount < bytesToRead; readCount++) {
        buffer[readCount] = readByte();
    }
    return readCount;
}

int Link::writeBytes(BYTE8* buffer, int bytesToWrite) {
    // TODO improve inefficiency
    int writtenCount = 0;
    for (; writtenCount < bytesToWrite; writtenCount++) {
        writeByte(buffer[writtenCount]);
    }
    return writtenCount;
}

WORD16 Link::readShort(void) {
    // Input a little-endian short, LSB first MSB last, forcing ordering of evaluation to prevent UB.
	BYTE8 lo = readByte();
	BYTE8 hi = readByte();
	return lo | (hi << 8);
}

void Link::writeShort(WORD16 w) {
    // Always output as a little-endian short, LSB first MSB last
    writeByte(w & 0x00ff);
    writeByte((w & 0xff00) >> 8);
}

WORD32 Link::readWord(void) {
	// Input a little-endian word, LSB first MSB last, forcing ordering of evaluation to prevent UB.
	BYTE8 b0 = readByte();
	BYTE8 b1 = readByte();
	BYTE8 b2 = readByte();
	BYTE8 b3 = readByte();
	return b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
}

void Link::writeWord(WORD32 w) {
	// Always output as a little-endian word, LSB first MSB last
	writeByte(w & 0x000000ff);
	writeByte((w & 0x0000ff00) >> 8);
	writeByte((w & 0x00ff0000) >> 16);
	writeByte((w & 0xff000000) >> 24);
}

int Link::getLinkNo(void) {
	return myLinkNo;
}

void Link::setDebug(bool newDebug) {
	bDebug = newDebug;
}

void Link::poll(std::function<void(WORD32)> reschedule) {
	// TODO
	// WORD32 w = readComplete();
	// if w != NotProcess.p
	//   reschedule(w)
	// w = writeComplete();
	// if w != NotProcess.p
	//   reschedule(w)
}

// New base classes - InterruptDrivenLink instead of AsyncLink holding the 'asynchronous' version of poll, below,
// then a new intermediate class SystemLink for holding the 'synchronous' version of this poll function, below,
// and an empty clock() method. The FIFOLink, NamedPipeLink, PicoUSBSerialLink, TTYLink, CommLink, would all be OperatingSystemLinks.
// GpioAsyncLink would be an InterruptDrivenLink. Link would be the superclass of them all.
//
// poll would only be used by the emulator, to handle reschedules.
// boot could continue to use the synchronous I/O methods, and the read availablilty method.
// The emulator must change IN, OUT/OUTBYTE/OUTWORD to use asynchronous I/O methods,
// and call poll at the end of the interpret loop for all links (so that rescheduling can happen).
//
// This implementation is only for SystemLink. InterruptDrivenLink version is simpler, see below.
//void Link::poll(std::function<void(WORD32)> reschedule) {
	// if readLinkRegister.workspace != NotProcess.p
	//   if readAvailable() // could be optimised to return the number of bytes available
	//     *readLinkRegister.buffer++ = readByte(); // similarly, block read/memory write?
	//     if --readLinkRegister.length == 0 // similarly, subtract amount read
	//       WORD32 t = readLinkRegister.workspace
	//       readLinkRegister.workspace = NotProcess.p
	//       readLinkRegister.buffer = nullptr
	//       reschedule(t)
	// if writeLinkRegister.workspace != NotProcess.p
	//   if writeAvailable()
	//     writeByte(*writeLinkRegister.buffer++)
	//     if --writeLinkRegister.length == 0
	//       WORD32 t = writeLinkRegister.workspace
	//       writeLinkRegister.workspace = NotProcess.p
	//       writeLinkRegister.buffer = nullptr
	//       reschedule(t)
//}
// InterruptDrivenLink version of poll:
// void InterruptDrivenLink::poll(std::function<void(WORD32)> reschedule) {
// WORD32 w = readComplete() // takes MUTEX
// if w != NotProcess.p
//   reschedule(w)
// w = writeComplete() // takes MUTEX
// if w != NotProcess.p
//   reschedule(w)

