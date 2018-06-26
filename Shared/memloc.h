//------------------------------------------------------------------------------
//
// File        : memloc.h
// Description : Special memory locations
// License     : GNU GPL - see COPYING for more details
// Created     : 09/09/2005
// Revision    : $Revision $
//
// (C) 2005 Matt J. Gumbley
// matt@gumbley.me.uk
// http://www.gumbley.me.uk/parachute
//
//------------------------------------------------------------------------------

#ifndef _MEMLOC_H
#define _MEMLOC_H

// Memory ranges
#define InternalMemStart 0x80000000
#define InternalMemEnd 0x80000FFF
#define ExternalMemStart 0x80001000
#define MemStart 0x80000070

// Special memory addresses
#define EregIntSaveLoc 0x80000044
#define StatusIntSaveLoc 0x80000040
#define CregIntSaveLoc 0x8000003C
#define BregIntSaveLoc 0x80000038
#define AregIntSaveLoc 0x80000034
#define IptrIntSaveLoc 0x80000030
#define WdescIntSaveLoc 0x8000002C
#define TPtrLoc0 0x80000028
#define TPtrLoc1 0x80000024
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

