//------------------------------------------------------------------------------
//
// File        : commlink.cpp
// Description : Implementation of a Link that uses a Windows COM port.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 17/06/2026
//
// (C) 2005-2026 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <string>
#include <stdexcept>
#include "types.h"
#include "link.h"
#include "commlink.h"
#include "log.h"
#include "misc.h"

CommLink::CommLink(int linkNo, bool isServer, const std::string &comPortName) :
    Link(linkNo, isServer) {
    logDebugF("Constructing COM link %d for %s", myLinkNo, isServer ? "server" : "cpu client");
    myHandle = nullptr;
    myWriteSequence = myReadSequence = 0;
    myComName = comPortName;
    myMsgbuf[0] = 0;
}

void CommLink::initialise() {
    myWriteSequence = myReadSequence = 0;
    logDebugF("Opening %s read/write", myComName.c_str());
    // Tries to mirror the setup done in ttylink.cpp for Linux/macOS/POSIX.
    // 8N1, CRTSCTS, non-canonical, IGNBRK|IGNPAR, VMIN=0/VTIME=20 (2-second
    // total read timeout).
    myHandle = CreateFileA(
        myComName.c_str(),
        GENERIC_READ | GENERIC_WRITE,   // Read/write
        0,                              // No sharing
        NULL,                           // No security
        OPEN_EXISTING,                  // Open existing port only
        0,                              // Non overlapped I/O
        NULL);                          // Null for comm devices
    logDebugF("Opened %s read/write, handle=%d", myComName.c_str(), myHandle);
    if (myHandle == INVALID_HANDLE_VALUE) {
        snprintf(myMsgbuf, COM_MSGBUF_SIZE, "Could not open COM %s: %s", myComName.c_str(), GetLastErrorStdStr().c_str());
        logWarn(myMsgbuf);
        throw std::runtime_error(myMsgbuf);
    }

    // DCB mirrors c_cflag / c_lflag / c_iflag / c_oflag.
    DCB dcb{};
    ZeroMemory(&dcb, sizeof(dcb));
    dcb.DCBlength = sizeof(DCB);

    if (!GetCommState(myHandle, &dcb)) {
        snprintf(myMsgbuf, COM_MSGBUF_SIZE, "GetCommState for COM %s failed: %s", myComName.c_str(), GetLastErrorStdStr().c_str());
        logWarn(myMsgbuf);
        throw std::runtime_error(myMsgbuf);
    }
    // USB CDC – nominal baud rate (matches B115200 in ttylink); 8 data bits; 1 stop bit.
    dcb.BaudRate = CBR_115200;
    dcb.ByteSize = 8;
    // No parity - IGNPAR makes parity irrelevant, but be explicit.
    dcb.Parity = NOPARITY;
    dcb.fParity = FALSE;
    dcb.StopBits = ONESTOPBIT;

    // CREAD – enable receiver. Must always be TRUE - also suppresses EOF/error processing
    // (closest to IGNBRK + raw mode).
    dcb.fBinary = TRUE;

    // CRTSCTS – hardware flow control.
    dcb.fOutxCtsFlow = TRUE;
    dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;

    // Disable software flow control (c_iflag has no IXON/IXOFF).
    dcb.fOutX = FALSE;
    dcb.fInX = FALSE;

    // IGNPAR equivalent – discard bytes with errors rather than
    // substituting a null; fErrorChar=FALSE + fNull=FALSE is the
    // closest Windows offers (no PARMRK-style marking).
    dcb.fErrorChar = FALSE;
    dcb.fNull = FALSE; // don't discard null bytes (raw)

    // DSR/DTR – leave DTR asserted, ignore DSR (CLOCAL equivalent).
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
    dcb.fDsrSensitivity = FALSE;
    dcb.fOutxDsrFlow = FALSE;

    // Abort-on-error off – let us keep reading after a framing/overrun.
    dcb.fAbortOnError = FALSE;

    if (!SetCommState(myHandle, &dcb)) {
        snprintf(myMsgbuf, COM_MSGBUF_SIZE, "SetCommState for COM %s failed: %s", myComName.c_str(), GetLastErrorStdStr().c_str());
        logWarn(myMsgbuf);
        throw std::runtime_error(myMsgbuf);
    }

    // COMMTIMEOUTS (mirrors VMIN=0 / VTIME=20, i.e. 2-second
    // total timeout with no inter-byte deadline)
    // POSIX VMIN=0/VTIME=N: return whatever is available after N*0.1 s,
    // or immediately if data arrives sooner.
    // ReadTotalTimeoutConstant    = 2000 ms  (VTIME 20 × 100 ms)
    //
    // Write timeouts: set a generous constant so a stalled Pico doesn't
    // hang the caller forever.
    COMMTIMEOUTS timeouts{};
    timeouts.ReadIntervalTimeout = 0;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant = 2000;   // VTIME 20 → 2 000 ms
    timeouts.WriteTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 2000;  // generous write timeout

    if (!SetCommTimeouts(myHandle, &timeouts)) {
        snprintf(myMsgbuf, COM_MSGBUF_SIZE, "SetCommTimeouts for COM %s failed: %s", myComName.c_str(), GetLastErrorStdStr().c_str());
        logWarn(myMsgbuf);
        throw std::runtime_error(myMsgbuf);
    }

    // Flush any stale data in the driver buffers, mirrors tcflush.
    PurgeComm(myHandle, PURGE_RXCLEAR | PURGE_TXCLEAR);
}

CommLink::~CommLink() {
    logDebugF("Destroying COM link %d", myLinkNo);
    if (myHandle != nullptr) {
        CloseHandle(myHandle);
        myHandle = nullptr;
    }
}

BYTE8 CommLink::readByte() {
    BYTE8 buf;
    DWORD readlen = 0;
	logDebugF("Reading byte from COM link %d", myLinkNo);
    while (readlen == 0) {
        if (ReadFile(myHandle, &buf, 1, &readlen, NULL)) {
            if (readlen == 1) {
                if (bDebug) {
                    logDebugF("Link %d R #%08X %02X (%c)", myLinkNo, myReadSequence++, buf, isprint(buf) ? buf : '.');
                }
                return buf;
            } else {
                // repeat
            }
        } else {
            snprintf(myMsgbuf, COM_MSGBUF_SIZE, "Could not read a byte from COM %s: (read %ld byte(s)) %s", myComName.c_str(), readlen, GetLastErrorStdStr().c_str());
            logWarn(myMsgbuf);
            readlen = 0;
        }
    }
    return 0;
}

void CommLink::writeByte(BYTE8 buf) {
    BYTE8 bufstore = buf;
    DWORD writelen = 0;
    if (bDebug) {
        logDebugF("Link %d W #%08X %02X (%c)", myLinkNo, myWriteSequence++, buf, isprint(buf) ? buf : '.');
    }
    if (WriteFile(myHandle, &bufstore, 1, &writelen, NULL)) {
        if (writelen == 1) {
            return;
        }
    }
    snprintf(myMsgbuf, COM_MSGBUF_SIZE, "Could not write a byte from COM %s: (wrote %ld byte(s)) %s", myComName.c_str(), writelen, GetLastErrorStdStr().c_str());
    logWarn(myMsgbuf);
    throw std::runtime_error(myMsgbuf);
}

void CommLink::resetLink() {
    PurgeComm(myHandle, PURGE_RXCLEAR | PURGE_TXCLEAR);
}

int CommLink::getLinkType() {
    return LinkType_TTY; // Even though it's a Windows COM port.
}
