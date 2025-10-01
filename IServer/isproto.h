//------------------------------------------------------------------------------
//
// File        : isproto.h
// Description : IServer protocol definition
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 31/08/2005
//
// (C) 2005-2025 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------
// This protocol is used between users of the libiserver.a client library
// (source for which can be found in the client directory), and the iserver
// process (source is in the server directory).
//
// The iserver is the interface point between code running on the
// emulator, and the host OS. The only other aspects of the host computer that
// emulated code has access to are the links, and the currently allocated
// memory area.
//
// Access to any other resource - console, chroot-jail filesystem area,
// notification of updates to the display surface, mouse events, etc. -
// is performed via the iserver protocol, which is an extended version of the
// INMOS iserver protocol.
//
// Accordingly, there are bits defined in the message type for each major
// subsystem accessible via the iserver.
//
// Access to the iserver is serialised - only one request can be
// outstanding at once. This is guaranteed by the libiserver code
// maintaining a common semaphore.
//
// Some messages may be oneway; some require a response.
//
// -----------------------------------------------------------------------------
//
// Protocol version history
// ========================
// Version Major changes
// ------- -------------
// 0       Initial version.
//------------------------------------------------------------------------------

#ifndef _ISPROTO_H
#define _ISPROTO_H

#include "types.h"

// Generic protocol request format on the wire is:
// +-----------------+
// | BYTE8 lenLsb    | Least significant byte of the 16-bit frame length.
// | BYTE8 lenMsb    | Most significant byte of the 16-bit frame length.
// +-----------------+
// | BYTE8 reqType   | Indicates the type of protocol message that follows. See REQ_xxx..
// +-----------------+
// | BYTE8 ........  | data pertinent to the message type, an undifferentiated
// |                 | byte stream.

// Frame length encoded in the first two bytes (little endian).
// Minimum frame length is 8 (ie minimum message length of 6 bytes in the to-server
// direction), maximum 512. Packet size must always be an even number of bytes.
// (TDS 2nd ed, p356, sec 16.5.1.)
// Inmos iServer has max packet size of 1040 ? Is that from T9000?

// STRING encoding in a frame is:
// +-----------------+
// | WORD16 length   | Number of bytes that follow in the String [0 .. StringBufferSize] (StringBufferSize=508)
// | BYTE8 .. chars  | <length> bytes of String character data
// +-----------------+

// Generic protocol response format on the wire is:
// +-----------------+
// | BYTE8 lsbSize   | Least significant byte of the size of the rest of the frame (always even)
// | BYTE8 msbSize   | Most significant byte of the size of the rest of the frame (always even)
// +-----------------+
// | BYTE8 resType   | one of the response type identifiers e.g. RES_SUCCESS
// +-----------------+
// | rest of data..  | the contents of this 'rest of data' is described for each
// | if any..        | specific response frame, below.
// +-----------------+


// File/Console handling

// Note for future extension: 9 is used as an 'unimplemented' value in tests.

const BYTE8 REQ_OPEN = 10;
const BYTE8 REQ_CLOSE = 11;
const BYTE8 REQ_READ = 12;
const BYTE8 REQ_WRITE = 13;
const BYTE8 REQ_GETS = 14;
const BYTE8 REQ_PUTS = 15;
const BYTE8 REQ_FLUSH = 16;
const BYTE8 REQ_SEEK = 17;
const BYTE8 REQ_TELL = 18;
const BYTE8 REQ_EOF = 19;
const BYTE8 REQ_FERROR = 20;
const BYTE8 REQ_REMOVE = 21;
const BYTE8 REQ_RENAME = 22;
const BYTE8 REQ_GETBLOCK = 23;
const BYTE8 REQ_PUTBLOCK = 24;
const BYTE8 REQ_ISATTY = 25;
const BYTE8 REQ_OPENREC = 26;
const BYTE8 REQ_GETREC = 27;
const BYTE8 REQ_PUTREC = 28;
const BYTE8 REQ_PUTEOF = 29;
const BYTE8 REQ_GETKEY = 30;
const BYTE8 REQ_POLLKEY = 31;

const BYTE8 REQ_GETENV = 32;
const BYTE8 REQ_TIME = 33;
const BYTE8 REQ_SYSTEM = 34;
const BYTE8 REQ_EXIT = 35;

const BYTE8 REQ_COMMAND = 40;
const BYTE8 REQ_CORE = 41;
const BYTE8 REQ_ID = 42; // a.k.a. 'Version'
const BYTE8 REQ_GETINFO = 43;

const BYTE8 REQ_MSDOS = 50;

const BYTE8 REQ_FILEEXISTS = 80;
const BYTE8 REQ_TRANSLATE = 81;
const BYTE8 REQ_FERRSTAT = 82;
const BYTE8 REQ_COMMANDARG = 83;

// DevZendo.org Extended IServer requests not present in the INMOS IServer
const BYTE8 REQ_PUTCHAR = 90;

// Result codes
const BYTE8 RES_SUCCESS = 0;
const BYTE8 RES_UNIMPLEMENTED = 1;
const BYTE8 RES_ERROR = 129;
const BYTE8 RES_NOPRIV = 131;
const BYTE8 RES_NORESOURCE = 132;
const BYTE8 RES_NOFILE = 133;
const BYTE8 RES_TRUNCATED = 134;
const BYTE8 RES_BADID = 135;
const BYTE8 RES_NOPOSN = 136;
const BYTE8 RES_NOTAVAILABLE = 137;
const BYTE8 RES_EOF = 138;
const BYTE8 RES_AKEYREPLY = 139;
const BYTE8 RES_BADPARAMS = 141;
const BYTE8 RES_NOTERM = 142;
const BYTE8 RES_RECTOOBIG = 143;


// REQ_OPEN
// Request:
// +----------------------+
// | BYTE8 REQ_OPEN       |
// +----------------------+
// | STRING filename      | Filename on the server to open
// | BYTE8 type           | File type, one of REQ_OPEN_TYPE below
// | BYTE8 mode           | Open mode, one of REQ_OPEN_MODE below
// +----------------------+
// Note: 'success' causes the IServer to exit with a success exit code (0); 'failure' exits it with
// a failure exit code (1). Other values are used directly as the IServer exit code.
//
// Response data for REQ_OPEN:
// +----------------------+
// | BYTE8 result         | RES_SUCCESS
// +----------------------+
// | WORD32 streamid      | A stream identifier for this open file
// +----------------------+
const BYTE8 REQ_OPEN_TYPE_BINARY = 1;
const BYTE8 REQ_OPEN_TYPE_TEXT = 2;
const BYTE8 REQ_OPEN_TYPE_VARIABLE = 3;
const BYTE8 REQ_OPEN_TYPE_FIXED = 4;

const BYTE8 REQ_OPEN_MODE_INPUT = 1;
const BYTE8 REQ_OPEN_MODE_OUTPUT = 2;
const BYTE8 REQ_OPEN_MODE_APPEND = 3;
const BYTE8 REQ_OPEN_MODE_EXISTING_UPDATE = 4;
const BYTE8 REQ_OPEN_MODE_NEW_UPDATE = 5;
const BYTE8 REQ_OPEN_MODE_APPEND_UPDATE = 6;


// REQ_READ
// Reads 'count' bytes of data from stream 'streamId'. Input stops when the specified number of bytes are read, or the
// end of stream is reached, or an error occurs. If 'count' is less than one, then no input is performed. The stream
// is left positioned immediately after the data read. If an error occurs then the stream position is undefined.
//
// Request:
// +----------------------+
// | BYTE8 REQ_READ       |
// +----------------------+
// | WORD32 streamid      | A stream identifier for this open file
// | WORD16 count         | Length of data to read
// +----------------------+
//
// Response data for REQ_READ:
// +----------------------+
// | BYTE8 result         | RES_SUCCESS on success; RES_BADID if stream not open, or out of range.
// +----------------------+
// | WORD16 count         | Length of data read, which may be less than requested. Use REQ_FEOF/REQ_FERROR to check.
// | BYTE8[] data         | A number of bytes read - see String definition, above - can be binary, not just ASCII.
// +----------------------+

// REQ_WRITE
// Writes 'count' bytes of binary data to stream 'streamId', which should be open for output. If 'count' is less than
// zero then no output is performed. The position of the stream is advanced by the number of bytes actually written.
// If an error occurs then the stream position is undefined.
//
// Request:
// +----------------------+
// | BYTE8 REQ_WRITE      |
// +----------------------+
// | WORD32 streamid      | A stream identifier for this open file
// | WORD16 count         | Length of data to write
// | BYTE8[] data         | A number of bytes to write - see String definition, above - can be binary, not just ASCII.
// +----------------------+
//
// Response data for REQ_WRITE:
// +----------------------+
// | BYTE8 result         | RES_SUCCESS on success; RES_BADID if stream not open, or out of range.
// +----------------------+
// | WORD16 count         | Length of data written, which may be less than requested. Use REQ_FEOF/REQ_FERROR to check.
// +----------------------+

// REQ_GETS
// TBC

// REQ_PUTS
// Writes a line of text to a stream 'streamId' which must be open for output. The host-specified convention for newline
// will be appended to the line and output to the file. The maximum line length is host-specified.
//
// Request:
// +----------------------+
// | BYTE8 REQ_PUTS       |
// +----------------------+
// | WORD32 streamid      | A stream identifier for this open file
// | WORD16 count         | Length of data to write
// | BYTE8[] data         | A number of bytes to write - see String definition, above - can be binary, not just ASCII.
// +----------------------+
//
// Response data for REQ_PUTS:
// +----------------------+
// | BYTE8 result         | RES_SUCCESS on success; RES_BADID if stream not open, or out of range.
// +----------------------+



// REQ_EXIT
// Request:
// +----------------------+
// | BYTE8 REQ_EXIT       |
// +----------------------+
// | WORD32 status        | a status code to send to the server; 999999999 is 'success'; -999999999 is 'failure'
// +----------------------+
// Note: 'success' causes the IServer to exit with a success exit code (0); 'failure' exits it with
// a failure exit code (1). Other values are used directly as the IServer exit code.
//
// Response data for REQ_EXIT:
// +----------------------+
// | BYTE8 result         | RES_SUCCESS
// +----------------------+
const WORD32 RES_EXIT_SUCCESS = 999999999;
const WORD32 RES_EXIT_FAILURE = -999999999;


// REQ_COMMAND
// Requests the command line arguments of the IServer. Either the full command
// line, or only those arguments that were not intended for the IServer itself.
// This is how a Transputer program (e.g. the occam compiler) gets its
// arguments: they're given to the IServer, which ignores them but allows the
// compiler to obtain them.
//
// Request:
// +----------------------+
// | BYTE8 REQ_COMMAND    |
// +----------------------+
// | BYTE8 all            | Zero requests only the Transputer program args.
// |                      | Nonzero requests the entire IServer command line
// |                      | including the name of the IServer command.
// +----------------------+
//
// Response data for REQ_COMMAND:
// +----------------------+
// | BYTE8 result         | RES_SUCCESS on success.
// | STRING commandline   | The command line.
// +----------------------+





// REQ_ID
// Request:
// +----------------------+
// | BYTE8 REQ_ID         |
// +----------------------+
//
// Response data for REQ_ID:
// +----------------------+
// | BYTE8 result         | RES_SUCCESS
// +----------------------+
// | BYTE8 version        | (major * 10) + (minor / 10)
// | BYTE8 host           | 0=unknown; 1=PC (Windows or Linux); 9=Mac
// | BYTE8 os             | 0=unknown; 6=Windows; 7=macOS; 8=Linux
// +----------------------+


// DevZendo.org Extended IServer requests

// REQ_PUTCHAR
// Writes a single character of text to the output console stream which must be open for output. 
//
// Request:
// +----------------------+
// | BYTE8 REQ_PUTCHAR    |
// +----------------------+
// | BYTE8 char           | A single character to write - can be binary, not just ASCII.
// +----------------------+
//
// Response data for REQ_PUTCHAR:
// +----------------------+
// | BYTE8 result         | RES_SUCCESS on success; RES_BADID if stream not open, or out of range.
// +----------------------+

#endif // _ISPROTO_H

