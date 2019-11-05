//------------------------------------------------------------------------------
//
// File        : isproto.h
// Description : IServer protocol definition
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 31/08/2005
//
// (C) 2005-2019 Matt J. Gumbley
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

// Protocol format on the wire is:
// +----------------+
// | WORD16 lenLsb  | Least significant byte of the 16-bit frame length.
// | WORD16 lenMsb  | Most significant byte of the 16-bit frame length.
// +----------------+
// | BYTE8 tag      | Indicates the type of protocol message that follows. See REQ_xxx..
// +----------------+
// | BYTE8 ........ | data pertinent to the message type, an undifferentiated
// |                | byte stream.

// Frame length encoded in the first two bytes (little endian).
// Minimum frame length is 8 (ie minimum message length of 6 bytes in the to-server
// direction), maximum 512. Packet size must always be an even number of bytes.
// (TDS 2nd ed, p356, sec 16.5.1.)
// Inmos iServer has max packet size of 1040 ? Is that from T9000?

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


// I'm using definitions rather than constants as it yields code that does less
// workspace thrashing.

// Current protocol version identifier
#define NS_PROTOCOL_VERSION 0

// Subsystem identifiers
#define NSS_SERVER 0x0000
#define NSS_CONSOLE 0x0100
#define NSS_FILESYSTEM 0x0200
#define NSS_DISPLAY 0x300
#define NSS_MOUSE 0x400
#define NSS_TIME 0x500
#define NSS_LITLOAD 0xAAAA0000

// Protocol format on the wire is:
// +----------------+
// | WORD32 msgType | one of the message type identifiers e.g. CONSOLE_PUT_CHAR
// +----------------+
// | BYTE ......... | data pertinent to the message type, an undifferentiated
// |                | byte stream.

//------------------------------------------------------------------------------
// Server protocol
#define SERVER_GET_VERSION (NSS_SERVER | 0x00)
// This is a request/response pair to obtain the version of the Node Server
// protocol in use.
// Request:
// +-------------------------------+
// | WORD32 SERVER_GET_VERSION     |
// +-------------------------------+
// Response:
// +-------------------------------+
// | WORD32 versionNumber          |
// +-------------------------------+

#define SERVER_EXIT (NSS_SERVER | 0x01)
// This is oneway request to the server to tell it to quit.
// +-------------------------------+
// | WORD32 SERVER_EXIT            |
// +-------------------------------+

//------------------------------------------------------------------------------
// Console protocol
#define CONSOLE_PUT_CHAR (NSS_CONSOLE | 0x00)
// This is oneway request to the server to output a single ASCII character.
// Currently this is passed straight through to the Node Server standard
// output channel.
// +-------------------------------+
// | WORD32 CONSOLE_PUT_CHAR       |
// +-------------------------------+
// | BYTE ASCIIChar                |
// +-------------------------------+

#define CONSOLE_PUT_PSTR (NSS_CONSOLE | 0x01)
// "Pascal String"
// This is oneway request to the server to display a counted-length String.
// Currently, this is passed straight through to the Node Server standard
// output channel.
// +-------------------------------+
// | WORD32 CONSOLE_PUT_PSTR       | PSTR Strings are limited to 255 chars
// +-------------------------------+
// | BYTE strLength                |
// | BYTE stringData[strLength]    |
// +-------------------------------+

#define CONSOLE_PUT_CSTR (NSS_CONSOLE | 0x02)
// "C String"
// This is oneway request to the server to display a NULL-terminated String.
// Currently, this is passed straight through to the Node Server standard
// output channel.
// This is oneway to the server.
// FIXME: the node server currently limits the amount printed to 16KB
// as defined here:
const int CONSOLE_PUT_CSTR_BUF_LIMIT=16384;
// +-------------------------------+
// | WORD32 CONSOLE_PUT_CSTR       | CSTR Strings are unlimited in length,
// +-------------------------------+ but cannot contain NUL characters
// | BYTE stringData ...           |
// | BYTE NULL                     |
// +-------------------------------+

#define CONSOLE_PUT_AVAILABLE (NSS_CONSOLE | 0x03)
// This is a request/response pair to enquire whether there is currently an
// output destination defined for the console. (e.g. Node Server standard 
// output channel or framebuffer console)
// Request:
// +-------------------------------+
// | WORD32 CONSOLE_PUT_AVAILABLE  |
// +-------------------------------+
// Response:
// +-------------------------------+
// | BYTE 0|1                      | 0=> unavailable, 1=> available
// +-------------------------------+

#define CONSOLE_GET_AVAILABLE (NSS_CONSOLE | 0x04)
// This is a request/response pair to enquire whether there is currently an
// input source defined for the console and some input to retrieve.
// (e.g. Node Server standard input channel or keyboard event source 
// received by framebuffer console window)
// Request:
// +-------------------------------+
// | WORD32 CONSOLE_GET_AVAILABLE  |
// +-------------------------------+
// Response:
// +-------------------------------+
// | BYTE 0|1                      | 0=> unavailable, 1=> available
// +-------------------------------+

#define CONSOLE_GET_CHAR (NSS_CONSOLE | 0x05)
// This is a request/response pair to retrieve the next input character from
// the console. (e.g. Node Server standard input channel or keyboard event 
// source received by framebuffer console window)
// Request:
// +-------------------------------+
// | WORD32 CONSOLE_GET_CHAR       |
// +-------------------------------+
// Response:
// +-------------------------------+
// | BYTE ASCIIChar                |
// +-------------------------------+


//------------------------------------------------------------------------------
// Time protocol
#define TIME_GET_MILLIS (NSS_TIME | 0x00)
// This is a request/response pair to retrieve the current date/time from the
// host OS in the form of milliseconds since midnight, January 1, 1970.
// Request:
// +-------------------------------+
// | WORD32 TIME_GET_MILLIS        |
// +-------------------------------+
// Response:
// +-------------------------------+
// | WORD32 millisSinceEpoch       |
// +-------------------------------+

#define TIME_GET_UTC (NSS_TIME | 0x01)
// This is a request/response pair to retrieve the current date/time from the
// host OS in the form of a structure describing the day, month, year, etc.
// Request:
// +-------------------------------+
// | WORD32 TIME_GET_UTC           |
// +-------------------------------+
// Response:
// +-------------------------------+
// | WORD32 dayInMonth [1..31]     |
// | WORD32 monthInYear [1..12]    |
// | WORD32 year (e.g. 1970, 2005) |
// | WORD32 hourInDay [0..23]      |
// | WORD32 minuteInHour [0..59]   |
// | WORD32 secondInMinute [0..61] |
// | WORD32 millisInSecond [0..999]|
// +-------------------------------+
// NB: secondInMinute can be up to 61 to allow for leap seconds.

#endif // _ISPROTO_H

