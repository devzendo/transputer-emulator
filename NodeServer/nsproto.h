//------------------------------------------------------------------------------
//
// File        : nsproto.h
// Description : Node Server protocol definition
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 31/08/2005
//
// (C) 2005-2018 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------
// This protocol is used between users of the libnodeserver.a client library
// (source for which can be found in the client directory), and the node
// server process (source is in the server directory).
//
// The node server is the interface point between code running on the
// emulator, and the host OS. The only other aspects of the host computer that
// emulated code has access to are the links, and the currently allocated
// memory area. Control over these is done via the OS library, which uses
// custom opcodes to effect these changes in the emulator.
//
// Access to any other resource - console, chroot-jail filesystem area,
// notification of updates to the display surface, mouse events, etc. -
// is performed via the node server protocol.
//
// Accordingly, there are bits defined in the message type for each major
// subsystem accessible via the node server.
//
// Access to the node server is serialised - only one request can be
// outstanding at once. This is guaranteed by the libnodeserver code
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

#ifndef _NSPROTO_H
#define _NSPROTO_H

#include "types.h"

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
// host OS in the form of milliseconds snce midnight, January 1, 1970.
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
// host OS in the form of a structure describing the day,month, year, etc.
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

#endif // _NSPROTO_H

