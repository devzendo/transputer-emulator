//------------------------------------------------------------------------------
//
// File        : opcodes.h
// Description : The transputer instruction set
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 29/06/2005
//
// (C) 2005-2018 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _OPCODES_H
#define _OPCODES_H

#include "emuquery.h"

// Direct and prefixing instructions
const int D_j = 0x00;
const int D_ldlp = 0x10;
const int D_pfix = 0x20;
const int D_ldnl = 0x30;
const int D_ldc = 0x40;
const int D_ldnlp = 0x50;
const int D_nfix = 0x60;
const int D_ldl = 0x70;
const int D_adc = 0x80;
const int D_call = 0x90;
const int D_cj = 0xa0;
const int D_ajw = 0xb0;
const int D_eqc = 0xc0;
const int D_stl = 0xd0;
const int D_stnl = 0xe0;
const int D_opr = 0xf0;

// Operations (Indirect instructions)
const int O_rev = 0x00;
const int O_add = 0x05;
const int O_sub = 0x0c;
const int O_mul = 0x53;
const int O_div = 0x2c;
const int O_rem = 0x1f;
const int O_sum = 0x52;
const int O_diff = 0x04;
const int O_prod = 0x08;
const int O_and = 0x46;
const int O_or = 0x4b;
const int O_xor = 0x33;
const int O_not = 0x32;
const int O_shl = 0x41;
const int O_shr = 0x40;
const int O_gt = 0x09;
const int O_lend = 0x21;
const int O_bcnt = 0x34;
const int O_wcnt = 0x3f;
const int O_ldpi = 0x1b;
const int O_mint = 0x42;
const int O_bsub = 0x02;
const int O_wsub = 0x0a;
const int O_move = 0x4a;
const int O_in = 0x07;
const int O_out = 0x0b;
const int O_lb = 0x01;
const int O_sb = 0x3b;
const int O_outbyte = 0x0e;
const int O_outword = 0x0f;
const int O_gcall = 0x06;
const int O_gajw = 0x3c;
const int O_ret = 0x20;
const int O_startp = 0x0d;
const int O_endp = 0x03;
const int O_runp = 0x39;
const int O_stopp = 0x15;
const int O_ldpri = 0x1e;
const int O_ldtimer = 0x22;
const int O_tin = 0x2b;
const int O_alt = 0x43;
const int O_altwt = 0x44;
const int O_altend = 0x45;
const int O_talt = 0x4e;
const int O_taltwt = 0x51;
const int O_enbs = 0x49;
const int O_diss = 0x30;
const int O_enbc = 0x48;
const int O_disc = 0x2f;
const int O_enbt = 0x47;
const int O_dist = 0x2e;
const int O_csub0 = 0x13;
const int O_ccnt1 = 0x4d;
const int O_testerr = 0x29;
const int O_stoperr = 0x55;
const int O_seterr = 0x10;
const int O_xword = 0x3a;
const int O_cword = 0x56;
const int O_xdble = 0x1d;
const int O_csngl = 0x4c;
const int O_ladd = 0x16;
const int O_lsub = 0x38;
const int O_lsum = 0x37;
const int O_ldiff = 0x4f;
const int O_lmul = 0x31;
const int O_ldiv = 0x1a;
const int O_lshl = 0x36;
const int O_lshr = 0x35;
const int O_norm = 0x19;
const int O_resetch = 0x12;
const int O_testpranal = 0x2a;
const int O_sthf = 0x18;
const int O_stlf = 0x1c;
const int O_sttimer = 0x54;
const int O_sthb = 0x50;
const int O_stlb = 0x17;
const int O_saveh = 0x3e;
const int O_savel = 0x3d;
const int O_clrhalterr = 0x57;
const int O_sethalterr = 0x58;
const int O_testhalterr = 0x59;
const int O_fmul = 0x72;
const int O_unpacksn = 0x63;
const int O_roundsn = 0x6d;
const int O_postnormsn = 0x6c;
const int O_ldinf = 0x71;
const int O_cflerr = 0x73;
const int O_dup = 0x5a;
const int O_move2dinit = 0x5b;
const int O_move2dall = 0x5c;
const int O_move2dnonzero = 0x5d;
const int O_move2dzero = 0x5e;
const int O_crcword = 0x74;
const int O_crcbyte = 0x75;
const int O_bitcnt = 0x76;
const int O_bitrevword = 0x77;
const int O_bitrevnbits = 0x78;
const int O_wsubdb = 0x81;
// Some floating point instructions are defined by
// loading Areg and using fpentry. See FP_ codes...
// See page 50 of Compiler Writer's guide. 
const int O_fpentry = 0xab;
const int O_fpdup = 0xa3;
const int O_fprev = 0xa4;
const int O_fpldnlsn = 0x8e;
const int O_fpldnldb = 0x8a;
const int O_fpldnlsni = 0x86;
const int O_fpldnldbi = 0x82;
const int O_fpstnlsn = 0x88;
const int O_fpstnldb = 0x84;
const int O_fpadd = 0x87;
const int O_fpsub = 0x89;
const int O_fpmul = 0x8b;
const int O_fpdiv = 0x8c;
const int O_fpremfirst = 0x8f;
const int O_fpremstep = 0x90;
const int O_fpldzerosn = 0x9f;
const int O_fpldzerodb = 0xa0;
const int O_fpldnladdsn = 0xaa;
const int O_fpldnladddb = 0xa6;
const int O_fpldnlmulsn = 0xac;
const int O_fpldnlmuldb = 0xa8;
const int O_fpchkerr = 0x83;
const int O_fptesterr = 0x9c;
const int O_fpgt = 0x94;
const int O_fpeq = 0x95;
const int O_fpordered = 0x92;
const int O_fpnan = 0x91;
const int O_fpnotfinite = 0x93;
const int O_fpint = 0xa1;
const int O_fpstnli32 = 0x9e;
const int O_fprtoi32 = 0x9d;
const int O_fpi32tor32 = 0x96;
const int O_fpi32tor64 = 0x98;
const int O_fpb32tor64 = 0x9a;

// Floating point operations: loaded into Areg, then fpentry
const int FP_fpusqrtfirst = 0x01;
const int FP_fpusqrtstep = 0x02;
const int FP_fpusqrtlast = 0x03;
const int FP_fpurp = 0x04;
const int FP_fpurm = 0x05;
const int FP_fpurz = 0x06;
const int FP_fpur32tor64 = 0x07;
const int FP_fpur64tor32 = 0x08;
const int FP_fpuexpdec32 = 0x09;
const int FP_fpuexpinc32 = 0x0a;
const int FP_fpuabs = 0x0b;
const int FP_fpunoround = 0x0d;
const int FP_fpchki32 = 0x0e;
const int FP_fpuchki64 = 0x0f;
const int FP_fpudivby2 = 0x11;
const int FP_fpumulby2 = 0x12;
const int FP_fpurn = 0x22;
const int FP_fpuseterr = 0x23;
const int FP_fpuclrerr = 0x9c;

// Nonstandard emulator functions
const int X_togglemonitor = 0xc0;
const int X_toggledisasm = 0xc1;
const int X_terminate = 0xc2;
const int X_marker = 0xc3;
const int X_emuquery = 0xc4;
// Emulator query operations: loaded into Areg, then X_emuquery, result on register stack
// EQA_ constants from emuquery.h in Shared
const int EQ_memtop = EQA_MEMTOP;


#endif // _OPCODES_H

