//------------------------------------------------------------------------------
//
// File        : fifolink.cpp
// Description : Implementation of link that uses a FIFO.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 19/07/2005
//
// (C) 2005-2019 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <exception>
#include <stdexcept>
#include <sys/types.h>
#include <sys/stat.h>

#include "platformdetection.h"

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "types.h"
#include "constants.h"
#include "link.h"
#include "fifolink.h"
#include "log.h"

FIFOLink::FIFOLink(int linkNo, bool isServer) : Link(linkNo, isServer) {
	logDebugF("Constructing FIFO link %d for %s", myLinkNo, isServer ? "server" : "cpu client");
	myWriteFD = -1;
	myReadFD = -1;
	myWriteSequence = myReadSequence = 0;
}

void FIFOLink::initialise(void) throw (std::exception) {
	static char msgbuf[255];
	struct stat st;
	// Filenames are relative to the CPU client.
	// The CPU client reads on the read FIFO and writes on the write FIFO.
	// The server reads on the write FIFO and writes on the read FIFO.
	// READ FIFO
	sprintf(myReadFifoName, "/tmp/t800emul-read-%d", myLinkNo);
	if (stat(myReadFifoName, &st) == -1) {
		// make the fifo
		if (mkfifo(myReadFifoName, 0600) == -1) {
			sprintf(msgbuf, "Could not create read FIFO %s: %s", myReadFifoName, strerror(errno));
			throw std::runtime_error(msgbuf);
		}
		if (stat(myReadFifoName, &st) == -1) {
			sprintf(msgbuf, "Could not obtain details of read FIFO %s: %s", myReadFifoName, strerror(errno));
			throw std::runtime_error(msgbuf);
		}
	}
	if (!S_ISFIFO(st.st_mode)) {
		sprintf(msgbuf, "Read FIFO file %s is not a FIFO", myReadFifoName);
		throw std::runtime_error(msgbuf);
	}
	// WRITE FIFO
	sprintf(myWriteFifoName, "/tmp/t800emul-write-%d", myLinkNo);
	if (stat(myWriteFifoName, &st) == -1) {
		// make the fifo
		if (mkfifo(myWriteFifoName, 0600) == -1) {
			sprintf(msgbuf, "Could not create write FIFO %s: %s", myWriteFifoName, strerror(errno));
			throw std::runtime_error(msgbuf);
		}
		if (stat(myWriteFifoName, &st) == -1) {
			sprintf(msgbuf, "Could not obtain details of write FIFO %s: %s", myWriteFifoName, strerror(errno));
			throw std::runtime_error(msgbuf);
		}
	}
	if (!S_ISFIFO(st.st_mode)) {
		sprintf(msgbuf, "Write FIFO file %s is not a FIFO", myWriteFifoName);
		throw std::runtime_error(msgbuf);
	}
	// Now open the FIFOs, using the FDs that will be used appropriately
	// for read/write given that the user is a server/CPU client.
	// Note that Linux guarantees that pipes opened with O_RDWR will not
	// block on open. Posix leaves this undefined. See fifo(4).
	if (bServer) {
		logDebugF("Opening %s write-only", myReadFifoName);
		myWriteFD = open(myReadFifoName, O_RDWR);
		if (myWriteFD == -1) {
			sprintf(msgbuf, "Could not open write FIFO %s: %s", myReadFifoName, strerror(errno));
			throw std::runtime_error(msgbuf);
		}
		logDebugF("Opening %s read-only", myWriteFifoName);
		myReadFD = open(myWriteFifoName, O_RDWR);
		if (myReadFD == -1) {
			sprintf(msgbuf, "Could not open read FIFO %s: %s", myWriteFifoName, strerror(errno));
			throw std::runtime_error(msgbuf);
		}
	} else {
		logDebugF("Opening %s read-only", myReadFifoName);
		myReadFD = open(myReadFifoName, O_RDWR);
		if (myReadFD == -1) {
			sprintf(msgbuf, "Could not open read FIFO %s: %s", myReadFifoName, strerror(errno));
			throw std::runtime_error(msgbuf);
		}
		logDebugF("Opening %s write-only", myWriteFifoName);
		myWriteFD = open(myWriteFifoName, O_RDWR);
		if (myWriteFD == -1) {
			sprintf(msgbuf, "Could not open write FIFO %s: %s", myWriteFifoName, strerror(errno));
			throw std::runtime_error(msgbuf);
		}
	}
}

FIFOLink::~FIFOLink() {
	logDebugF("Destroying FIFO link %d", myLinkNo);
	if (myReadFD != -1) {
		close(myReadFD);
		myReadFD = -1;
	}
	if (myWriteFD != -1) {
		close(myWriteFD);
		myWriteFD = -1;
	}
	// It's not fatal if we can't remove the FIFOs, as the first side (emulator/server) that gets here will do it.
    logDebugF("Removing %s", myReadFifoName);
	if (unlink(myReadFifoName) == -1) {
        logDebugF("Could not remove %s: %s", myReadFifoName, strerror(errno));
    }
    logDebugF("Removing %s", myWriteFifoName);
    if (unlink(myWriteFifoName) == -1) {
        logDebugF("Could not remove %s: %s", myWriteFifoName, strerror(errno));
    }
}

BYTE8 FIFOLink::readByte() throw (std::exception) {
	static char msgbuf[255];
	BYTE8 buf;
	if (read(myReadFD, &buf, 1) == 1) {
		if (bDebug) {
			logDebugF("Link %d R #%08X %02X (%c)", myLinkNo, myReadSequence++, buf, isprint(buf) ? buf : '.');
		}
		return buf;
	}
	sprintf(msgbuf, "Could not read a byte from FIFO FD#%d: %s", myReadFD, strerror(errno));
	logWarn(msgbuf);
	throw std::runtime_error(msgbuf);
}

void FIFOLink::writeByte(BYTE8 buf) throw (std::exception) {
	static char msgbuf[255];
	BYTE8 bufstore = buf;
	if (bDebug) {
		logDebugF("Link %d W #%08X %02X (%c)", myLinkNo, myWriteSequence++, buf, isprint(buf) ? buf : '.');
	}
	if (write(myWriteFD, &bufstore, 1) == 1) {
		return;
	}
	sprintf(msgbuf, "Could not write a byte to FIFO FD#%d: %s", myWriteFD, strerror(errno));
	throw std::runtime_error(msgbuf);
}

void FIFOLink::resetLink(void) throw (std::exception) {
	// TODO
}

