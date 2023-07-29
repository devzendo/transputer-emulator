//------------------------------------------------------------------------------
//
// File        : flags.h
// Description : bit settings for flags emulator state and debug flags
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 12/7/05005
//
// (C) 2005-2023 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _FLAGS_H
#define _FLAGS_H

/* The Flags bits hold the following debug settings.
   Some of the options set in the config file are stored here, and may be
   changed by some parts of the emulator. 
   
   Flags is a 32 bit integer, partitioned as follows:

   Bit(s) & Mask      Function
   ===========================
   Emulator Configuration Flags
   
   0 (0xF)            
   1                   Holds the degugging level of the emulator.
   2                   There are 16 possible levels, as given below.
   3
   
   4 (0x30)            Memory access debug levels. Up to 4 levels.
   5
   
   6 (0x40)            Link communications debug: on or off
   
   7 (0x80)            IServer Diagnostics: on or off
   
   8 (0x100)           Clock pulse diagnostics: on or off
   
   9 (0x200)           Process queue diagnostics: on or off
   
   10 (0x400)          Terminate on a memory violation: on or off
   
   11 (0x800)          Monitor active
   
   12 (0x1000)         Reserved
   
   13 (0x2000)         Reserved
   
   14 (0x4000)         Reserved
   
   15 (0x8000)         Reserved
*/

// Masks for determining settings of the individual DebugFlags
#define DebugFlags_DebugLevel 0xF
#define DebugFlags_MemAccessDebugLevel 0x30
#define DebugFlags_LinkComms 0x40
#define DebugFlags_IDiag 0x80
#define DebugFlags_Clocks 0x100
#define DebugFlags_Queues 0x200
#define DebugFlags_TerminateOnMemViol 0x0400
#define DebugFlags_Monitor 0x0800

// Debugging Levels, i.e. flags & DebugFlags_DebugLevel
#define Debug_None 0                            // No debugging information
#define Debug_Disasm 1        // A disassembly is produced during emulation
#define Debug_DisRegs 2                // A disassembly with register dumps
#define Debug_OprCodes 3    // As above, but with decoding of opr / fpentry

// Memory access debug levels
#define MemAccessDebug_No 0                // No logging of memory accesses
#define MemAccessDebug_ReadWriteData 0x10       // Reading and Writing data
#define MemAccessDebug_Full 0x20          // Read/Write data / Instructions



// Global flags variable
extern WORD32 flags; 

// Flag mask applied to Flags to turn off all debugging
#define DebugFlagMask (~(DebugFlags_DebugLevel | \
                         DebugFlags_MemAccessDebugLevel | \
                         DebugFlags_LinkComms | DebugFlags_Clocks | \
                         DebugFlags_Queues | DebugFlags_IDiag))

/* There are also the following emulator state bit settings:

   Bit(s) & Mask      Function
   ===========================
   Emulator State Flags
   
   16 (0x10000)        T800 Error flag
   
   17 (0x20000)        T800 Halt on Error flag
   
   18 (0x40000)        T800 Floating point Error flag
   
   19 (0x80000)        Emulator: Deschedule pending due to timeslice
   
   20 (0x100000)       Emulator: Deschedule required by instruction
  
   21 (0x200000)       Emulator: Interruptible instruction interpreted
  
   22 (0x400000)       Emulator: Bad / unimplemented instruction encountered
  
   23 (0x800000)       Emulator: Queue manipulation instruction encountered
  
   24 (0x1000000)      Emulator: Timer manipulation instruction encountered

   25 (0x2000000)      Emulator: Breakpoint instruction (j 0, break) encountered
  
   26 (0x4000000)      T800 Break on j 0 flag
  
   27 (0x8000000)      Reserved
  
   28 (0x10000000)     Reserved
   
   29 (0x20000000)     Reserved
  
   30 (0x40000000)     Reserved
  
   31 (0x80000000)     Emulator: termination required
*/

#define EmulatorState_ErrorFlag 0x10000
#define EmulatorState_HaltOnError 0x20000
#define EmulatorState_FErrorFlag 0x40000
#define EmulatorState_DeschedulePending 0x80000
#define EmulatorState_DescheduleRequired 0x100000
#define EmulatorState_Interrupt 0x200000
#define EmulatorState_BadInstruction 0x400000
#define EmulatorState_QueueInstruction 0x800000
#define EmulatorState_TimerInstruction 0x1000000
#define EmulatorState_BreakpointInstruction 0x2000000
#define EmulatorState_J0Break 0x4000000

#define EmulatorState_Terminate 0x80000000

// Flag mask applied before each instruction, to reset some of the flags:
#define FlagMask  (~(EmulatorState_DescheduleRequired | \
                     EmulatorState_BadInstruction | \
                     EmulatorState_TimerInstruction | \
                     EmulatorState_QueueInstruction | \
                     EmulatorState_Interrupt))

// Macros for testing, setting and clearing flags:
#define IS_FLAG_SET(test)       (flags & (test))
#define IS_FLAG_CLEAR(test)     (!(flags & (test)))
#define SET_FLAGS(set)          flags |= (set)
#define CLEAR_FLAGS(clear)      flags &= (~(clear))


#endif // _FLAGS_H

