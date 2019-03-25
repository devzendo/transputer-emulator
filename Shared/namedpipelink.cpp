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
	logDebugF("Constructing named pipe link %d for %s", myLinkNo, isServer ? "server" : "cpu client");
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
    logDebugF("Read named pipe called %s", myReadPipeName);

    if (bServer) {
        logDebugF("Server creating named pipe %s", myReadPipeName);
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
        logDebugF("Client opening named pipe %s", myReadPipeName);
        myReadHandle = CreateFile(
                myReadPipeName, // pipe name
                GENERIC_READ |  // read and write access
                GENERIC_WRITE,
                0,              // no sharing
                NULL,           // default security attributes
                OPEN_EXISTING,  // opens existing pipe
                0,              // default attributes
                NULL);          // no template file
    }

    if (myReadHandle == INVALID_HANDLE_VALUE) {
        sprintf_s(msgbuf, "Could not create/open read named pipe: Error %d", GetLastError());
        throw std::runtime_error(msgbuf);
    }
    logDebug("Read named pipe created");



    // WRITE NAMED PIPE
    sprintf_s(myWritePipeName, NAME_LEN, "\\\\.\\pipe\\t800emul-write-%d", myLinkNo);
    logDebugF("Write named pipe called %s", myWritePipeName);

    if (bServer) {
        logDebugF("Server creating named pipe %s", myWritePipeName);
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
        logDebugF("Client opening named pipe %s", myWritePipeName);
        myWriteHandle = CreateFile(
                myWritePipeName, // pipe name
                GENERIC_READ |   // read and write access
                GENERIC_WRITE,
                0,               // no sharing
                NULL,            // default security attributes
                OPEN_EXISTING,   // opens existing pipe
                0,               // default attributes
                NULL);           // no template file
    }

    if (myWriteHandle == INVALID_HANDLE_VALUE) {
        sprintf_s(msgbuf, "Could not create/open write named pipe: Error %d", GetLastError());
        throw std::runtime_error(msgbuf);
    }
    logDebug("Write named pipe created");
}

NamedPipeLink::~NamedPipeLink() {
    if (myConnected) {
        if (bServer) {
            logDebugF("Server disconnecting from named pipe link %d", myLinkNo);
            if (DisconnectNamedPipe(myReadHandle)) {
                logDebug("Disconnected");
            } else {
                logWarnF("Failed to disconnect from pipe %s: Error %d", myReadPipeName, GetLastError());
            }
        } else {
            // Just closing (below) is sufficient.
        }
        myConnected = false;
    }

	logDebugF("Destroying named pipe link %d", myLinkNo);
	if (myReadHandle != INVALID_HANDLE_VALUE) {
	    CloseHandle(myReadHandle);
	    myReadHandle = INVALID_HANDLE_VALUE;
	}
	if (myWriteHandle != INVALID_HANDLE_VALUE) {
	    CloseHandle(myWriteHandle);
	    myWriteHandle = INVALID_HANDLE_VALUE;
	}
}

void NamedPipeLink::connect(void) throw (std::exception) {
    if (myConnected) {
        return;
    }

    if (bServer) {
        logDebugF("Server connecting to named pipe %s", myReadPipeName);
        // Wait for the client to connect; if it succeeds,
        // the function returns a nonzero value. If the function
        // returns zero, GetLastError returns ERROR_PIPE_CONNECTED.
        if (ConnectNamedPipe(myReadHandle, NULL) ? true : (GetLastError() == ERROR_PIPE_CONNECTED)) {
            logDebug("Connected");
        } else {
            logWarnF("Failed to connect to pipe %s", myReadPipeName);
            throw std::runtime_error("Failed to connect to pipe");
        }
    } else {
    }

    myConnected = true;
}

BYTE NamedPipeLink::readByte() throw (std::exception) {
    static char msgbuf[255];
    BYTE buf;
    DWORD cbBytesRead = 0, cbReplyBytes = 0, cbWritten = 0;
    BOOL fSuccess = FALSE;

    connect();

    fSuccess = ReadFile(
            myReadHandle, // handle to pipe
            &buf,         // buffer to receive data
            1,            // size of buffer
            &cbBytesRead, // number of bytes read
            NULL);        // not overlapped I/O

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

    connect();

    if (bDebug) {
        logDebugF("Link %d W #%08X %02X (%c)", myLinkNo, myWriteSequence++, buf, isprint(buf) ? buf : '.');
    }

    fSuccess = WriteFile(
            myWriteHandle, // pipe handle
            &bufstore,     // message
            1,             // message length
            &cbWritten,    // bytes written
            NULL);         // not overlapped

    if (!fSuccess)
    {
        sprintf_s(msgbuf, "Could not write a byte to named pipe %s: Miscellaneous error %d", myWritePipeName, GetLastError());
        logWarn(msgbuf);
        throw std::runtime_error(msgbuf);
    }
}

void NamedPipeLink::resetLink(void) throw (std::exception) {
	// TODO
}

