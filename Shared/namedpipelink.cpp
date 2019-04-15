//------------------------------------------------------------------------------
//
// File        : namedpipelink.cpp
// Description : Implementation of link that uses a Windows named pipe.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 05/03/2019
//
// (C) 2005-2019 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <exception>
#include <stdexcept>
#include <windows.h>
#include <winbase.h>
#include <fileapi.h>

#include "platformdetection.h"

#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "types.h"
#include "constants.h"
#include "link.h"
#include "namedpipelink.h"
#include "log.h"

const int BUFSIZE = 512;

NamedPipeLink::NamedPipeLink(int linkNo, bool isServer) : Link(linkNo, isServer) {
	logDebugF("[CTOR] Constructing named pipe link %d for %s", myLinkNo, isServer ? "server" : "cpu client");
    myWriteHandle = INVALID_HANDLE_VALUE;
    myReadHandle = INVALID_HANDLE_VALUE;
    myWriteSequence = myReadSequence = 0;
    myReadPipeName[0] = '\0';
    myWritePipeName[0] = '\0';
}

void NamedPipeLink::initialise(void) throw (std::exception) {
	static char msgbuf[255];

	// Filenames are relative to the CPU client.
	// The CPU client reads on the read FIFO and writes on the write FIFO.
    // The server reads on the write FIFO and writes on the read FIFO.
	// READ NAMED PIPE
    sprintf_s(myReadPipeName, NAME_LEN, "\\\\.\\pipe\\t800emul-read-%d", myLinkNo);
    logDebugF("[init] Read named pipe called %s", myReadPipeName);

    if (bServer) {
        logDebugF("[init] Server creating named pipe %s", myReadPipeName);
        myReadHandle = CreateNamedPipe(
                myReadPipeName,           // pipe name
                PIPE_ACCESS_DUPLEX,       // read/write access
                PIPE_TYPE_MESSAGE |       // message type pipe
                PIPE_READMODE_MESSAGE |   // message-read mode
                PIPE_WAIT,                // blocking mode
                PIPE_UNLIMITED_INSTANCES, // max. instances
                BUFSIZE,                  // output buffer size
                BUFSIZE,                  // input buffer size
                0,                        // client time-out
                NULL);                    // default security attribute
    } else {
    }

    if (myReadHandle == INVALID_HANDLE_VALUE) {
        sprintf_s(msgbuf, "Could not create/open read named pipe: Error %d", GetLastError());
        throw std::runtime_error(msgbuf);
    }
    logDebug("[init] Read named pipe created");



    // WRITE NAMED PIPE
    sprintf_s(myWritePipeName, NAME_LEN, "\\\\.\\pipe\\t800emul-write-%d", myLinkNo);
    logDebugF("[init] Write named pipe called %s", myWritePipeName);

    if (bServer) {
        logDebugF("[init] Server creating named pipe %s", myWritePipeName);
        myWriteHandle = CreateNamedPipe(
                myWritePipeName,          // pipe name
                PIPE_ACCESS_DUPLEX,       // read/write access
                PIPE_TYPE_MESSAGE |       // message type pipe
                PIPE_READMODE_MESSAGE |   // message-read mode
                PIPE_WAIT,                // blocking mode
                PIPE_UNLIMITED_INSTANCES, // max. instances
                BUFSIZE,                  // output buffer size
                BUFSIZE,                  // input buffer size
                0,                        // client time-out
                NULL);                    // default security attribute
    } else {
    }

    if (myWriteHandle == INVALID_HANDLE_VALUE) {
        sprintf_s(msgbuf, "Could not create/open write named pipe: Error %d", GetLastError());
        throw std::runtime_error(msgbuf);
    }
    logDebug("[init] Write named pipe created");
}

NamedPipeLink::~NamedPipeLink() {
    if (myConnected) {
        if (bServer) {
            logDebugF("[DTOR] Server disconnecting from named pipe link %d", myLinkNo);
            if (DisconnectNamedPipe(myReadHandle)) {
                logDebug("[DTOR] Disconnected");
            } else {
                logWarnF("[DTOR] Failed to disconnect from pipe %s: Error %d", myReadPipeName, GetLastError());
            }
        } else {
            // Just closing (below) is sufficient.
        }
        myConnected = false;
    }

	logDebugF("[DTOR] Destroying named pipe link %d", myLinkNo);
	if (myReadHandle != INVALID_HANDLE_VALUE) {
	    CloseHandle(myReadHandle);
	    myReadHandle = INVALID_HANDLE_VALUE;
	}
	if (myWriteHandle != INVALID_HANDLE_VALUE) {
	    CloseHandle(myWriteHandle);
	    myWriteHandle = INVALID_HANDLE_VALUE;
	}
    logDebugF("[DTOR] Destroyed named pipe link %d", myLinkNo);
}

void NamedPipeLink::connect(void) throw (std::exception) {
    if (myConnected) {
        return;
    }

    if (bServer) {
        logDebugF("[connect] Server connecting to named pipe %s", myReadPipeName);
        // Wait for the client to connect; if it succeeds,
        // the function returns a nonzero value. If the function
        // returns zero, GetLastError returns ERROR_PIPE_CONNECTED.
        if (ConnectNamedPipe(myReadHandle, NULL) ? true : (GetLastError() == ERROR_PIPE_CONNECTED)) {
            logDebug("[connect] Server connected");
        } else {
            logWarnF("[connect] Server failed to connect to pipe %s", myReadPipeName);
            throw std::runtime_error("Failed to connect to pipe");
        }
    } else {
        while (true) {
            logDebugF("[connect] Client opening read named pipe %s", myReadPipeName);
            myReadHandle = CreateFile(
                    myReadPipeName, // pipe name
                    GENERIC_READ |  // read and write access
                    GENERIC_WRITE,
                    0,              // no sharing
                    NULL,           // default security attributes
                    OPEN_EXISTING,  // opens existing pipe
                    0,              // default attributes
                    NULL);          // no template file
            if (myReadHandle != INVALID_HANDLE_VALUE) {
                break; // it's open
            }
            if (GetLastError() != ERROR_PIPE_BUSY) {
                logWarnF("[connect] Client could not open read named pipe. GLE=%d", GetLastError());
                throw std::runtime_error("Failed to open read named pipe in connect");
            }
            // All pipe instances are busy, so wait....
            logDebugF("[connect] Client waiting for server of read named pipe...");
            if (WaitNamedPipe(myReadPipeName, 1000)) // ms
            {
                logDebug("[connect] Client detected Server connected to read named pipe");
                break;
            }
        }

        while (true) {
            logDebugF("[connect] Client opening write named pipe %s", myWritePipeName);
            myWriteHandle = CreateFile(
                    myWritePipeName, // pipe name
                    GENERIC_READ |  // read and write access
                    GENERIC_WRITE,
                    0,              // no sharing
                    NULL,           // default security attributes
                    OPEN_EXISTING,  // opens existing pipe
                    0,              // default attributes
                    NULL);          // no template file
            if (myWriteHandle != INVALID_HANDLE_VALUE) {
                break; // it's open
            }
            if (GetLastError() != ERROR_PIPE_BUSY) {
                logWarnF("[connect] Client could not open write named pipe. GLE=%d", GetLastError());
                throw std::runtime_error("Failed to open write named pipe in connect");
            }
            // All pipe instances are busy, so wait....
            logDebugF("[connect] Client waiting for server of write named pipe...");
            if (WaitNamedPipe(myWritePipeName, 1000)) // ms
            {
                logDebug("[connect] Client detected Server connected to write named pipe");
                break;
            }
        }
    }
    logDebug("[connect] Connected");

    myConnected = true;
}

BYTE NamedPipeLink::readByte() throw (std::exception) {
    static char msgbuf[255];
    BYTE buf;
    DWORD cbBytesRead = 0, cbReplyBytes = 0, cbWritten = 0;
    BOOL fSuccess = FALSE;
    logDebugF("[readByte] Read byte on link %d by %s", myLinkNo, isServer ? "server" : "cpu client");

    connect();
    logDebugF("[readByte] ReadFile call by link %d by %s", myLinkNo, isServer ? "server" : "cpu client");

    fSuccess = ReadFile(
            myReadHandle, // handle to pipe
            &buf,         // buffer to receive data
            1,            // size of buffer
            &cbBytesRead, // number of bytes read
            NULL);        // not overlapped I/O
    logDebugF("[readByte] ReadFile return %s, bytes read=%d", fSuccess, cbBytesRead);

    if (!fSuccess || cbBytesRead == 0)
    {
        if (GetLastError() == ERROR_BROKEN_PIPE)
        {
            sprintf_s(msgbuf, "Could not read a byte from named pipe %s: Client disconnected/Broken Pipe", myReadPipeName);
        }
        else
        {
            sprintf_s(msgbuf, "Could not read a byte from named pipe %s: Miscellaneous error %d", myReadPipeName, GetLastError());
        }
        logWarn(msgbuf);
        throw std::runtime_error(msgbuf);
    }
    if (bDebug) {
        logDebugF("Link %d R #%08X %02X (%c)", myLinkNo, myReadSequence++, buf, isprint(buf) ? buf : '.');
    }
    return buf;
}

void NamedPipeLink::writeByte(BYTE buf) throw (std::exception) {
    static char msgbuf[255];

    BYTE bufstore = buf;
    DWORD cbBytesRead = 0, cbReplyBytes = 0, cbWritten = 0;
    BOOL fSuccess = FALSE;
    logDebugF("[writeByte] Write byte on link %d by %s", myLinkNo, isServer ? "server" : "cpu client");

    connect();

    if (bDebug) {
        logDebugF("Link %d W #%08X %02X (%c)", myLinkNo, myWriteSequence++, buf, isprint(buf) ? buf : '.');
    }
    logDebugF("[writeByte] WriteFile call by link %d by %s", myLinkNo, isServer ? "server" : "cpu client");

    fSuccess = WriteFile(
            myWriteHandle, // pipe handle
            &bufstore,     // message
            1,             // message length
            &cbWritten,    // bytes written
            NULL);         // not overlapped
    logDebugF("[writeByte] WriteFile return %s, bytes read=%d", fSuccess, cbWritten);

    if (!fSuccess)
    {
        sprintf_s(msgbuf, "Could not write a byte to named pipe %s: Miscellaneous error %d", myWritePipeName, GetLastError());
        logWarn(msgbuf);
        throw std::runtime_error(msgbuf);
    }
}

void NamedPipeLink::resetLink(void) throw (std::exception) {
	// TODO
    logDebugF("[resetLink] Reset link %d by %s", myLinkNo, isServer ? "server" : "cpu client");
}

