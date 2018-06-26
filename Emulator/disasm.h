//------------------------------------------------------------------------------
//
// File        : disasm.h
// Description : disassembly functions
// License     : GNU GPL - see COPYING for more details
// Created     : 20/07/2005
// Revision    : $Revision $
//
// (C) 2005 Matt J. Gumbley
// matt@gumbley.me.uk
// http://www.gumbley.me.uk/parachute
//
//------------------------------------------------------------------------------

#ifndef _DISASM_H
#define _DISASM_H
extern char *disassembleDirectOperation(WORD32 Instruction, WORD32 Oreg);
extern char *disassembleIndirectOperation(WORD32 Oreg, WORD32 Areg);
#endif // _DISASM_H

