//------------------------------------------------------------------------------
//
// File        : namedpipelink.cpp
// Description : Implementation of link that uses a Windows named pipe.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 05/03/2019
//
// (C) 2005-2025 Matt J. Gumbley
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
    myPipeHandle = INVALID_HANDLE_VALUE;
    myWriteSequence = myReadSequence = 0;
    myPipeName[0] = '\0';
}

void NamedPipeLink::initialise(void) {
	static char msgbuf[255];

    sprintf_s(myPipeName, NAME_LEN, "\\\\.\\pipe\\temulink%d", myLinkNo);
    logDebugF("[init] Named pipe link %d for %s called %s", myLinkNo, bServer ? "server" : "cpu client", myPipeName);

    if (bServer) {
        logDebugF("[init] Server creating named pipe %s", myPipeName);
        myPipeHandle = CreateNamedPipeA(
                myPipeName,               // pipe name
                PIPE_ACCESS_DUPLEX,       // read/write access
                PIPE_TYPE_BYTE |          // byte type pipe
                  PIPE_READMODE_BYTE |    // byte-read mode
                  PIPE_WAIT,              // blocking mode
                2,                        // max. instances
                BUFSIZE,                  // output buffer size
                BUFSIZE,                  // input buffer size
                0,                        // client time-out
                NULL);                    // default security attribute

        if (myPipeHandle == INVALID_HANDLE_VALUE) {
            sprintf_s(msgbuf, "Could not create/open named pipe: Error %d", GetLastError());
            throw std::runtime_error(msgbuf);
        }
        logDebug("[init] Named pipe created");
    } else {
        logDebugF("[init] Client waiting to read/write before connecting to named pipe %s", myPipeName);
    }
}

NamedPipeLink::~NamedPipeLink() {
    if (myConnected) {
        logDebugF("[DTOR] Flushing named pipe link %d", myLinkNo);
		FlushFileBuffers(myPipeHandle);
        if (bServer) {
            logDebugF("[DTOR] Server disconnecting from named pipe link %d", myLinkNo);
            if (DisconnectNamedPipe(myPipeHandle)) {
                logDebug("[DTOR] Disconnected");
            } else {
                logWarnF("[DTOR] Failed to disconnect from pipe %s: Error %d", myPipeName, GetLastError());
            }
        } else {
            // Just closing (below) is sufficient.
        }
        myConnected = false;
    }

	logDebugF("[DTOR] Destroying named pipe link %d", myLinkNo);
	if (myPipeHandle != INVALID_HANDLE_VALUE) {
	    CloseHandle(myPipeHandle);
	    myPipeHandle = INVALID_HANDLE_VALUE;
	}
    logDebugF("[DTOR] Destroyed named pipe link %d", myLinkNo);
}

void NamedPipeLink::connect(void) {
    if (myConnected) {
        return;
    }

    if (bServer) {
        logDebugF("[connect] Server connecting to named pipe %s", myPipeName);
        // Wait for the client to connect; if it succeeds,
        // the function returns a nonzero value. If the function
        // returns zero, GetLastError returns ERROR_PIPE_CONNECTED.
        if (ConnectNamedPipe(myPipeHandle, NULL) ? true : (GetLastError() == ERROR_PIPE_CONNECTED)) {
            logDebug("[connect] Server detected Client connected");
        } else {
            logWarnF("[connect] Server failed to detect client connect to pipe %s", myPipeName);
            throw std::runtime_error("Failed to connect to pipe");
        }
    } else {
        while (true) {
            logDebugF("[connect] Client opening named pipe %s", myPipeName);
            myPipeHandle = CreateFile(
                    myPipeName,     // pipe name
                    GENERIC_READ |  // read and write access
                      GENERIC_WRITE,
                    0,              // no sharing
                    NULL,           // default security attributes
                    OPEN_EXISTING,  // opens existing pipe
                    0,              // default attributes
                    NULL);          // no template file
            if (myPipeHandle != INVALID_HANDLE_VALUE) {
                logDebugF("[connect] Client opened named pipe %s; got handle %d", myPipeName, myPipeHandle);
                break; // it's open
            }
            if (GetLastError() != ERROR_PIPE_BUSY) {
                logWarnF("[connect] Client could not open named pipe. GLE=%d", GetLastError());
                throw std::runtime_error("Failed to open named pipe in connect");
            }
            // All pipe instances are busy, so wait....
            logDebugF("[connect] Client waiting for server of named pipe...");
            if (WaitNamedPipe(myPipeName, 1000)) // ms
            {
                logDebug("[connect] Client detected Server connected to named pipe");
                break;
            }
        }
    }
    logDebug("[connect] Connected");

    myConnected = true;
}

BYTE NamedPipeLink::readByte() {
    static char msgbuf[255];
    BYTE buf;
    DWORD cbBytesRead = 0, cbReplyBytes = 0, cbWritten = 0;
    BOOL fSuccess = FALSE;
    logDebugF("[readByte] Read byte on link %d by %s", myLinkNo, bServer ? "server" : "cpu client");

    connect();
    logDebugF("[readByte] ReadFile call by link %d by %s", myLinkNo, bServer ? "server" : "cpu client");

    fSuccess = ReadFile(
            myPipeHandle, // handle to pipe
            &buf,         // buffer to receive data
            1,            // size of buffer
            &cbBytesRead, // number of bytes read
            NULL);        // not overlapped I/O
    logDebugF("[readByte] ReadFile return %d, bytes read=%d", fSuccess, cbBytesRead);

    if (!fSuccess || cbBytesRead == 0)
    {
        if (GetLastError() == ERROR_BROKEN_PIPE)
        {
            sprintf_s(msgbuf, "Could not read a byte from named pipe %s: Client disconnected/Broken Pipe", myPipeName);
        }
        else
        {
            sprintf_s(msgbuf, "Could not read a byte from named pipe %s: Miscellaneous error %d", myPipeName, GetLastError());
        }
        logWarn(msgbuf);
        throw std::runtime_error(msgbuf);
    }
    if (bDebug) {
        logDebugF("Link %d R #%08X %02X (%c)", myLinkNo, myReadSequence++, buf, isprint(buf) ? buf : '.');
    }
    return buf;
}

void NamedPipeLink::writeByte(BYTE buf) {
    static char msgbuf[255];

    BYTE bufstore = buf;
    DWORD cbBytesRead = 0, cbReplyBytes = 0, cbWritten = 0;
    BOOL fSuccess = FALSE;
    logDebugF("[writeByte] Write byte on link %d by %s", myLinkNo, bServer ? "server" : "cpu client");

    connect();

    if (bDebug) {
        logDebugF("Link %d W #%08X %02X (%c)", myLinkNo, myWriteSequence++, buf, isprint(buf) ? buf : '.');
    }
    logDebugF("[writeByte] WriteFile call by link %d by %s with handle %d", myLinkNo, bServer ? "server" : "cpu client", myPipeHandle);

    fSuccess = WriteFile(
            myPipeHandle, // pipe handle
            &bufstore,     // message
            1,             // message length
            &cbWritten,    // bytes written
            NULL);         // not overlapped
    logDebugF("[writeByte] WriteFile return %d, bytes written=%d", fSuccess, cbWritten);

    if (!fSuccess)
    {
        sprintf_s(msgbuf, "Could not write a byte to named pipe %s: Miscellaneous error %d", myPipeName, GetLastError());
        logWarn(msgbuf);
        throw std::runtime_error(msgbuf);
    }
}

void NamedPipeLink::resetLink(void) {
	// TODO
    logDebugF("[resetLink] Reset link %d by %s", myLinkNo, bServer ? "server" : "cpu client");
}

int NamedPipeLink::getLinkType() {
    return LinkType_NamedPipe;
}
