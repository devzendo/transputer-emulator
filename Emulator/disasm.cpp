//------------------------------------------------------------------------------
//
// File        : disasm.cpp
// Description : disassembly functions
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 20/07/2005
//
// (C) 2005-2025 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <cstdio>
#include <cstring>
#include "types.h"
#include "constants.h"
#include "log.h"
#include "flags.h"
#include "disasm.h"
#include "opcodes.h"

static const char *disassembleDirectInstName(WORD32 Instruction) {
	switch (Instruction) {
		case D_pfix:
			return "pfix";
		case D_nfix:
			return "nfix";
		case D_j:
			return "j";
		case D_ldlp:
			return "ldlp";
		case D_ldnl:
			return "ldnl";
		case D_ldc:
			return "ldc";
		case D_ldnlp:
			return "ldnlp";
		case D_ldl:
			return "ldl";
		case D_adc:
			return "adc";
		case D_call:
			return "call";
		case D_cj:
			return "cj";
		case D_ajw:
			return "ajw";
		case D_eqc:
			return "eqc";
		case D_stl:
			return "stl";
		case D_stnl:
			return "stnl";
		default:
			return "?direct?";
	}
}

char *disassembleDirectOperation(WORD32 Instruction, WORD32 Oreg) {
	static char buf[255];
	buf[0] = '\0';
#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
	sprintf(buf, "%s #%08X", disassembleDirectInstName(Instruction), Oreg);
#elif defined(PLATFORM_WINDOWS)
	sprintf_s(buf, 255, "%s #%08X", disassembleDirectInstName(Instruction), Oreg);
#endif

	return buf;
}

static const char *disassembleIndirectInstName(WORD32 Oreg, WORD32 Areg) {
	switch (Oreg) {
		case O_rev:
			return "rev"; 
		case O_add:
			return "add"; 
		case O_sub:
			return "sub"; 
		case O_mul:
			return "mul"; 
		case O_div:
			return "div"; 
		case O_rem:
			return "rem"; 
		case O_sum:
			return "sum"; 
		case O_diff:
			return "diff"; 
		case O_prod:
			return "prod"; 
		case O_and:
			return "and"; 
		case O_or:
			return "or"; 
		case O_xor:
			return "xor"; 
		case O_not:
			return "not"; 
		case O_shl:
			return "shl"; 
		case O_shr:
			return "shr"; 
		case O_gt:
			return "gt"; 
		case O_lend:
			return "lend"; 
		case O_bcnt:
			return "bcnt"; 
		case O_wcnt:
			return "wcnt"; 
		case O_ldpi:
			return "ldpi"; 
		case O_mint:
			return "mint"; 
		case O_bsub:
			return "bsub"; 
		case O_wsub:
			return "wsub"; 
		case O_move:
			return "move"; 
		case O_in:
			return "in"; 
		case O_out:
			return "out"; 
		case O_lb:
			return "lb"; 
		case O_sb:
			return "sb"; 
		case O_outbyte:
			return "outbyte"; 
		case O_outword:
			return "outword"; 
		case O_gcall:
			return "gcall"; 
		case O_gajw:
			return "gajw"; 
		case O_ret:
			return "ret"; 
		case O_startp:
			return "startp"; 
		case O_endp:
			return "endp"; 
		case O_runp:
			return "runp"; 
		case O_stopp:
			return "stopp"; 
		case O_ldpri:
			return "ldpri"; 
		case O_ldtimer:
			return "ldtimer"; 
		case O_tin:
			return "tin"; 
		case O_alt:
			return "alt"; 
		case O_altwt:
			return "altwt"; 
		case O_altend:
			return "altend"; 
		case O_talt:
			return "talt"; 
		case O_taltwt:
			return "taltwt"; 
		case O_enbs:
			return "enbs"; 
		case O_diss:
			return "diss"; 
		case O_enbc:
			return "enbc"; 
		case O_disc:
			return "disc"; 
		case O_enbt:
			return "enbt"; 
		case O_dist:
			return "dist"; 
		case O_csub0:
			return "csub0"; 
		case O_ccnt1:
			return "ccnt1"; 
		case O_testerr:
			return "testerr"; 
		case O_stoperr:
			return "stoperr"; 
		case O_seterr:
			return "seterr"; 
		case O_xword:
			return "xword"; 
		case O_cword:
			return "cword"; 
		case O_xdble:
			return "xdble"; 
		case O_csngl:
			return "csngl"; 
		case O_ladd:
			return "ladd"; 
		case O_lsub:
			return "lsub"; 
		case O_lsum:
			return "lsum"; 
		case O_ldiff:
			return "ldiff"; 
		case O_lmul:
			return "lmul"; 
		case O_ldiv:
			return "ldiv"; 
		case O_lshl:
			return "lshl"; 
		case O_lshr:
			return "lshr"; 
		case O_norm:
			return "norm"; 
		case O_resetch:
			return "resetch"; 
		case O_testpranal:
			return "testpranal"; 
		case O_sthf:
			return "sthf"; 
		case O_stlf:
			return "stlf"; 
		case O_sttimer:
			return "sttimer"; 
		case O_sthb:
			return "sthb"; 
		case O_stlb:
			return "stlb"; 
		case O_saveh:
			return "saveh"; 
		case O_savel:
			return "savel"; 
		case O_clrhalterr:
			return "clrhalterr"; 
		case O_sethalterr:
			return "sethalterr"; 
		case O_testhalterr:
			return "testhalterr"; 
		case O_fmul:
			return "fmul"; 
		case O_unpacksn:
			return "unpacksn"; 
		case O_roundsn:
			return "roundsn"; 
		case O_postnormsn:
			return "postnormsn"; 
		case O_ldinf:
			return "ldinf"; 
		case O_cflerr:
			return "cflerr"; 
		case O_dup:
			return "dup"; 
		case O_move2dinit:
			return "move2dinit"; 
		case O_move2dall:
			return "move2dall"; 
		case O_move2dnonzero:
			return "move2dnonzero"; 
		case O_move2dzero:
			return "move2dzero"; 
		case O_crcword:
			return "crcword"; 
		case O_crcbyte:
			return "crcbyte"; 
		case O_bitcnt:
			return "bitcnt"; 
		case O_bitrevword:
			return "bitrevword"; 
		case O_bitrevnbits:
			return "bitrevnbits"; 
		case O_wsubdb:
			return "wsubdb"; 
		case O_fpdup:
			return "fpdup"; 
		case O_fprev:
			return "fprev"; 
		case O_fpldnlsn:
			return "fpldnlsn"; 
		case O_fpldnldb:
			return "fpldnldb"; 
		case O_fpldnlsni:
			return "fpldnlsni"; 
		case O_fpldnldbi:
			return "fpldnldbi"; 
		case O_fpstnlsn:
			return "fpstnlsn"; 
		case O_fpstnldb:
			return "fpstnldb"; 
		case O_fpadd:
			return "fpadd"; 
		case O_fpsub:
			return "fpsub"; 
		case O_fpmul:
			return "fpmul"; 
		case O_fpdiv:
			return "fpdiv"; 
		case O_fpremfirst:
			return "fpremfirst"; 
		case O_fpremstep:
			return "fpremstep"; 
		case O_fpldzerosn:
			return "fpldzerosn"; 
		case O_fpldzerodb:
			return "fpldzerodb"; 
		case O_fpldnladdsn:
			return "fpldnladdsn"; 
		case O_fpldnladddb:
			return "fpldnladddb"; 
		case O_fpldnlmulsn:
			return "fpldnlmulsn"; 
		case O_fpldnlmuldb:
			return "fpldnlmuldb"; 
		case O_fpchkerr:
			return "fpchkerr"; 
		case O_fptesterr:
			return "fptesterr"; 
		case O_fpgt:
			return "fpgt"; 
		case O_fpeq:
			return "fpeq"; 
		case O_fpordered:
			return "fpordered"; 
		case O_fpnan:
			return "fpnan"; 
		case O_fpnotfinite:
			return "fpnotfinite"; 
		case O_fpint:
			return "fpint"; 
		case O_fpstnli32:
			return "fpstnli32"; 
		case O_fprtoi32:
			return "fprtoi32"; 
		case O_fpi32tor32:
			return "fpi32tor32"; 
		case O_fpi32tor64:
			return "fpi32tor64"; 
		case O_fpb32tor64:
			return "fpb32tor64"; 
		case O_fpentry:
			switch (Areg) {
				case FP_fpusqrtfirst:
					return "fpusqrtfirst"; 
				case FP_fpusqrtstep:
					return "fpusqrtstep"; 
				case FP_fpusqrtlast:
					return "fpusqrtlast"; 
				case FP_fpurz:
					return "fpurz"; 
				case FP_fpurp:
					return "fpurp"; 
				case FP_fpurm:
					return "fpurm"; 
				case FP_fpur32tor64:
					return "fpur32tor64"; 
				case FP_fpur64tor32:
					return "fpur64tor32"; 
				case FP_fpuexpinc32:
					return "fpuexpinc32"; 
				case FP_fpuexpdec32:
					return "fpuexpdec32"; 
				case FP_fpuabs:
					return "fpuabs"; 
				case FP_fpunoround:
					return "fpunoround"; 
				case FP_fpchki32:
					return "fpchki32"; 
				case FP_fpuchki64:
					return "fpuchki64"; 
				case FP_fpudivby2:
					return "fpudivby2"; 
				case FP_fpumulby2:
					return "fpumulby2"; 
				case FP_fpurn:
					return "fpurn"; 
				case FP_fpuseterr:
					return "fpuseterr"; 
				case FP_fpuclrerr:
					return "fpuclrerr"; 
				default :
					return "?fp?";
			}
			break;
		// T801 instructions
		case O_start:
			return "start";
		case O_testhardchan:
			return "testhardchan";
		case O_testldd:
			return "testldd";
		case O_teststd:
			return "teststd";
		case O_testlde:
			return "testlde";
		case O_testste:
			return "testste";
		case O_testlds:
			return "testlds";
		case O_teststs:
			return "teststs";

		// T801/T805 instructions
		case O_break:
			return "break";
		case O_clrj0break:
			return "clrj0break";
		case O_setj0break:
			return "setj0break";
		case O_testj0break:
			return "testj0break";
		case O_timerdisableh:
			return "timerdisableh";
		case O_timerdisablel:
			return "timerdisablel";
		case O_timerenableh:
			return "timerenableh";
		case O_timerenablel:
			return "timerenablel";
		case O_ldmemstartval:
			return "ldmemstartval";
		case O_pop:
			return "pop";
		case O_lddevid:
			return "lddevid";

		// Nonstandard emulator
		case X_togglemonitor:
			return "togglemonitor";
		case X_toggledisasm:
			return "toggledisasm";
		case X_terminate:
			return "terminate";
		case X_marker:
			return "marker";
		case X_emuquery:
			return "emuquery";

		default:
			return "?indirect?";
	}
}


/* Disassemble an indirect operation. Oreg holds the operation code, and
   for the case of certain Floating Point operations, Areg holds an operation
   code.
 */
char *disassembleIndirectOperation(WORD32 Oreg, WORD32 Areg) {
	static char buf[255];
	buf[0] = '\0';
	if ((flags & DebugFlags_DebugLevel) >= Debug_OprCodes) {
#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
		sprintf(buf, " (O=#%08X) ", Oreg);
#elif defined(PLATFORM_WINDOWS)
		sprintf_s(buf, 255, " (O=#%08X) ", Oreg);
#endif
	}
	if (Oreg == O_fpentry) {
		if ((flags & DebugFlags_DebugLevel) >= Debug_OprCodes) {
#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
			sprintf(buf, " (fpentry A=#%08X) ", Areg);
#elif defined(PLATFORM_WINDOWS)
			sprintf_s(buf, 255, " (fpentry A=#%08X) ", Areg);
#endif
		} else {
#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
			strcat(buf, " ");
#elif defined(PLATFORM_WINDOWS)
			strcat_s(buf, 255, " ");
#endif
        }
    }
#if defined(PLATFORM_OSX) || defined(PLATFORM_LINUX)
	strcat(buf, disassembleIndirectInstName(Oreg, Areg));
#elif defined(PLATFORM_WINDOWS)
	strcat_s(buf, 255, disassembleIndirectInstName(Oreg, Areg));
#endif
	return buf;
}
