//------------------------------------------------------------------------------
//
// File        : memloc.h
// Description : Special memory locations
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 09/09/2005
//
// (C) 2005-2023 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _MEMLOC_H
#define _MEMLOC_H

// Memory addresses in the Transputer are signed, and extend from the most negative address
// 80000000 (-2147483648) to FFFFFFFF (-1) to the positive range 00000000 (0) to 7FFFFFFF (+2147483647)
// ROM (if present) extends from its first byte, until MaxINT - it is loaded at the end of memory, so that the
// 2-byte jump at ResetCode is present.

// Memory ranges
#define InternalMemStart 0x80000000
#define InternalMemEnd 0x80000FFF
#define ExternalMemStart 0x80001000
#define MemStart 0x80000070

#define MaxINT 0x7FFFFFFF

// Special memory addresses
#define ResetCode 0x7FFFFFFE
// 0x80000048 - 0x8000006c are "reserved for extended functions" - Transputer Reference Manual, p 74
#define EregIntSaveLoc 0x80000044
#define StatusIntSaveLoc 0x80000040
#define CregIntSaveLoc 0x8000003C
#define BregIntSaveLoc 0x80000038
#define AregIntSaveLoc 0x80000034
#define IptrIntSaveLoc 0x80000030
#define WdescIntSaveLoc 0x8000002C
#define TPtrLoc1 0x80000028
#define TPtrLoc0 0x80000024
#define Event 0x80000020
#define Link3Input 0x8000001C
#define Link2Input 0x80000018
#define Link1Input 0x80000014
#define Link0Input 0x80000010
#define Link3Output 0x8000000C
#define Link2Output 0x80000008
#define Link1Output 0x80000004
#define Link0Output 0x80000000

#endif // _MEMLOC_H

