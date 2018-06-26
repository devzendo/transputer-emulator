//------------------------------------------------------------------------------
//
// File        : fifolink.cpp
// Description : Implementation of link that uses a FIFO.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 19/07/2005
// Revision    : $Revision $
//
// (C) 2005-2018 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <exception>
#include <stdexcept>
using namespace std;
#include <sys/types.h>
#include <sys/stat.h>
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

void FIFOLink::initialise(void) throw (exception) {
	static char msgbuf[255];
	char rfname[80];
	char wfname[80];
	struct stat st;
	// Filenames are relative to the CPU client.
	// The CPU client reads on the read FIFO and writes on the write FIFO.
	// Any server reads on the write FIFO and reads on the read FIFO.
	// READ FIFO
	sprintf(rfname, "/tmp/t800emul-read-%d", myLinkNo);
	if (stat(rfname, &st) == -1) {
		// make the fifo
		if (mkfifo(rfname, 0600) == -1) {
			sprintf(msgbuf, "Could not create read FIFO %s: %s", rfname, strerror(errno));
			throw runtime_error(msgbuf);
		}
		if (stat(rfname, &st) == -1) {
			sprintf(msgbuf, "Could not obtain details of read FIFO %s: %s", rfname, strerror(errno));
			throw runtime_error(msgbuf);
		}
	}
	if (!S_ISFIFO(st.st_mode)) {
		sprintf(msgbuf, "Read FIFO file %s is not a FIFO", rfname);
		throw runtime_error(msgbuf);
	}
	// WRITE FIFO
	sprintf(wfname, "/tmp/t800emul-write-%d", myLinkNo);
	if (stat(wfname, &st) == -1) {
		// make the fifo
		if (mkfifo(wfname, 0600) == -1) {
			sprintf(msgbuf, "Could not create write FIFO %s: %s", wfname, strerror(errno));
			throw runtime_error(msgbuf);
		}
		if (stat(wfname, &st) == -1) {
			sprintf(msgbuf, "Could not obtain details of write FIFO %s: %s", wfname, strerror(errno));
			throw runtime_error(msgbuf);
		}
	}
	if (!S_ISFIFO(st.st_mode)) {
		sprintf(msgbuf, "Write FIFO file %s is not a FIFO", wfname);
		throw runtime_error(msgbuf);
	}
	// Now open the FIFOs, using the FDs that will be used appropriately
	// for read/write given that the user is a server/CPU client.
	// Note that Linux guarantees that pipes opened with O_RDWR will not
	// block on open. Posix leaves this undefined. See fifo(4).
	if (bServer) {
		logDebugF("Opening %s write-only", rfname);
		myWriteFD = open(rfname, O_RDWR);
		if (myWriteFD == -1) {
			sprintf(msgbuf, "Could not open write FIFO %s: %s", wfname, strerror(errno));
			throw runtime_error(msgbuf);
		}
		logDebugF("Opening %s read-only", wfname);
		myReadFD = open(wfname, O_RDWR);
		if (myReadFD == -1) {
			sprintf(msgbuf, "Could not open read FIFO %s: %s", rfname, strerror(errno));
			throw runtime_error(msgbuf);
		}
	} else {
		logDebugF("Opening %s read-only", rfname);
		myReadFD = open(rfname, O_RDWR);
		if (myReadFD == -1) {
			sprintf(msgbuf, "Could not open read FIFO %s: %s", rfname, strerror(errno));
			throw runtime_error(msgbuf);
		}
		logDebugF("Opening %s write-only", wfname);
		myWriteFD = open(wfname, O_RDWR);
		if (myWriteFD == -1) {
			sprintf(msgbuf, "Could not open write FIFO %s: %s", wfname, strerror(errno));
			throw runtime_error(msgbuf);
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
}

BYTE FIFOLink::readByte() throw (exception) {
	static char msgbuf[255];
	BYTE buf;
	if (read(myReadFD, &buf, 1) == 1) {
		if (bDebug) {
			logDebugF("Link %d R #%08X %02X (%c)", myLinkNo, myReadSequence++, buf, isprint(buf) ? buf : '.');
		}
		return buf;
	}
	sprintf(msgbuf, "Could not read a byte from FIFO FD#%d: %s", myReadFD, strerror(errno));
	logWarn(msgbuf);
	throw runtime_error(msgbuf);
}

void FIFOLink::writeByte(BYTE buf) throw (exception) {
	static char msgbuf[255];
	BYTE bufstore = buf;
	if (bDebug) {
		logDebugF("Link %d W #%08X %02X (%c)", myLinkNo, myWriteSequence++, buf, isprint(buf) ? buf : '.');
	}
	if (write(myWriteFD, &bufstore, 1) == 1) {
		return;
	}
	sprintf(msgbuf, "Could not write a byte to FIFO FD#%d: %s", myWriteFD, strerror(errno));
	throw runtime_error(msgbuf);
}

void FIFOLink::resetLink(void) throw (exception) {
	// TODO
}

