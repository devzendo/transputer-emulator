//------------------------------------------------------------------------------
//
// File        : disasm.h
// Description : disassembly functions
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 20/07/2005
//
// (C) 2005-2020 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _DISASM_H
#define _DISASM_H
extern char *disassembleDirectOperation(WORD32 Instruction, WORD32 Oreg);
extern char *disassembleIndirectOperation(WORD32 Oreg, WORD32 Areg);
#endif // _DISASM_H

