//------------------------------------------------------------------------------
//
// File        : constants.h
// Description : Various constants for the emulator.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 29/06/2005
//
// (C) 2005-2019 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _CONSTANTS_H
#define _CONSTANTS_H

#include "types.h"
#include "platformdetection.h"

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

// For storing bool values in WORD32s
const int BOOL_TRUE = 1;
const int BOOL_FALSE = 0;

const int Kilo=1024;
const int Mega=(Kilo * Kilo);
const int DefaultMemSize=(4 * Mega);          // Emulator has a 4MB address space
const int BitsPerWord=32;                       // For use in instruction timings
const int ByteSelectMask=0x03;                    // Mask to check word alignment
const WORD32 WordMask=0xFFFFFFFC;             // ~ByteSelectMask to get word part

const WORD32 NotProcess_p=0x80000000;  // Minimum integer; lowest memory location

// Values that the ALT state of an alternative, held in WPtr - 3 can have
const WORD32 Enabling_p=0x80000001;        // Indicates alt enabling is occurring
const WORD32 Waiting_p=0x80000002;              // Indicates alt time not set yet
const WORD32 Ready_p=0x80000003;                    // Indicates a guard is ready

// Values that the timer list link, held in WPtr - 4 can have
const WORD32 TimeSet_p=0x80000001;
const WORD32 TimeNotSet_p=0x80000002;

const WORD32 NoneSelected_o=0xFFFFFFFF;      // No branch selected in alt disable


const WORD32 SignBit=0x80000000;        // Location of the sign bit in a register
const int MaxQuantum=2048;          // Quantum of execution for low priority task 
                                    //      expressed in number of HiClock ticks.


#endif // _CONSTANTS_H

