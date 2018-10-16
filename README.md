transputer-emulator
===================
This is an emulator of the Inmos T805 Transputer, and Node Server that interfaces it
to a host OS, providing boot/debug/IO facilities. It is part of the
[Parachute Project](https://devzendo.github.io/parachute). 

(C) 2005-2018 Matt J. Gumbley
matt.gumbley@devzendo.org
@mattgumbley @devzendo
http://devzendo.github.io/parachute

Status
------
In progress, October 2018. Modernising, building on modern OSX/Linux/Windows.

Remaining work:
* Node Server client library in tmasm assembler
* Node server needs to support terminal I/O facilities (no echo key reads, sensing available readable keys) for eForth

Future intentions:
* Replace NodeServer with an iserver/afserver-protocol compatible version
* Add remaining T800/T801/T805 instructions, including those from Transputer Instruction Set - Appendix, Guy Harriman.
  esp pop - why not in T800?!
* Unfinished: resetch when given a link not a memory channel
* Remove potential buffer overflows in cpu.cpp (what were these?)
* Refactoring: make use of isLegalMemory within memory.cpp
* Build on Linuxes
* Build on Windows
* Fix all compiler warnings (some tautologous comparisons remain)
* Upgrade to C++11 or more recent
* Emulate multiple Transputers on 1-N physical cores
* Link topology management
* Investigate Benes networks
* Develop debug interface
* Write debugger program
* Add memory-mapped frame buffer via SDL
* Add mouse interface for same

Bugs:
* (possibly obviated by CMake) why does 'make test' in Shared says it fail to link, yet actually does, and works?

Completeness
------------
Although the emulator recognises all T805 instructions, not all are currently implemented.
The greatest omission is floating point - this is an integer T805 at the moment!

Those that are not emit a diagnostic when encountered.

See the sections 'Implemented Instructions' and 'Unimplemented Instructions' below.

It is my intention to eventually implement the complete instruction set. My focus is on the common integer instructions,
those that support communications, process management, diagnostics, and then the floating point instructions.

Implemented Instructions
========================
Direct:
adc, ajw, call, cj, eqc, j, ldc, ldl, ldlp, ldnl, ldnlp, nfix, opr, pfix, stl,
stnl.

Operations:
add, alt, altend, altwt, and, bcnt, bitclt, bitrevnbits, bitrevword, bsub,
ccnt1, clrhalterr, csngl, csub0, cword, diff, disc, diss, dist, div, dup, enbc,
enbs, enbt, endp, fpentry, fpuclrerr, fpuseterr, gajw, gcall, gt, in, ladd, lb,
ldiff, ldiv, ldpi, ldpri, ldtimer, lend, lmul, lshl, lshr, lsub, lsum, mint,
move, mul, not, or, out, outbyte, outword, prod, rem, ret, rev, runp, saveh,
savel, sb, seterr, sethalterr, shl, shr, startp, sthb, sthf, stlb, stlf,
stoperr, stopp, sttimer, sub, sum, talt, testerr, testhalterr, tin, wcnt, wsub,
wsubdb, xdble, xor, xword.

Nonstandard emulator:
emuquery, marker, terminate, toggledisasm, togglemonitor

Unfinished Instructions
=======================
fpchkerr, fptesterr, resetch, taltwt.

Unimplemented Instructions
==========================
cflerr, crcbyte, crcword, f puexpdec32, fmul, fpadd, fpb32tor64, fpchki32,
fpdiv, fpdup, fpeq, fpgt, fpi32tor32, fpi32tor64, fpint, fpldnladddb,
fpldnladdsn, fpldnldb, fpldnldbi, fpldnlmuldb, fpldnlmulsn, fpldnlsn,
fpldnlsni, fpldzerodb, fpldzerosn, fpmul, fpnan, fpnotfinite, fpordered,
fpremfirst, fpremstep, fprev, fprtoi32, fpstnldb, fpstnli32, fpstnlsn, fpsub,
fpuabs, fpuchki64, fpudivby2, fpuexpinc32, fpumulby2, fpunoround, fpur32tor64,
fpur64tor32, fpurm, fpurn, fpurp, fpurz, fpusqrtfirst, fpusqrtlast,
fpusqrtstep, ldinf, move2dall, move2dinit, move2dnonzero, move2dzero, norm,
postnormsn, roundsn, testpranal, unpacksn.

T801:
start, testhardchan, testldd, testlde, testlds, teststd, testste, teststs.



Release Notes
-------------
0.01 (ongoing work for the first release)
* Started adding the T801 instructions, from "Transputer Instruction Set - Appendix, Guy Harriman", and ""
* Builds using CMake/CLion on OSX.
* Added Boot-from-ROM, fixed Wdesc bug after boot from link.
* Fixes: xword, call, j & scheduling (with assistance from Michael Brüstle), locations of TPtrLoc1, TPtrLoc0.
* Monitor: db (renamed from da), dw improvements, added w (workspace display), added b/b+b-/b? (breakpoints),
  added s (show all state).
* Adopted the Apache License v2.


Using the Emulator
------------------
Read the manual in Documentation/Manual (Requires OpenOffice/LibreOffice).


Directory structure
-------------------
The source is split into the following directories:

Shared - utility code that is common to many parts of the system.

NodeServer - the client and server portions of the node server and its protocol
definition. The server runs on your host computer (i.e. under Windows, Linux,
Mac OS X etc.). The client runs under T800 emulation as part of your
application; it's an assembly language include file.

Emulator - the T800 emulator.


Building and Installing
-----------------------
The distribution currently builds under the following systems:
* Mac OSX 'El Capitan' 10.11

Later I intend to provide buils for:
* Ubuntu Linux 16.04.4 LTS Intel x86-64
* CentOS Linux 7.4 Intel x86-64
* Windows 10

Prerequisites:
- All: <a href="https://bitbucket.org/devzendo/transputer-macro-assembler">DevZendo.org
       Transputer Macro Assembler</a> installed and on your PATH
       GNU Make.
- OSX: clang (e.g. via XCode Developer tools, or MacPorts)
- Ubuntu Linux: build-essential (=> gcc)
- CentOS: gcc
- Windows: (future work, but you'll need a UNIXy toolchain)


The install location is /opt/parachute. Changing this would entail changes to
Makefiles in the above hierarchy.

To build, cd to the top level directory (where this README.md is) and do:
make

During the build, you will be prompted for a password. This is since the files
are installed as root, and the build requires a sudo password.

This build will build the entire system: T800 emulator and node
server, client libraries, etc.



License
-------
This code is released under the Apache 2.0 License: http://www.apache.org/licenses/LICENSE-2.0.html.
(C) 2005-2018 Matt Gumbley, DevZendo.org


Acknowledgements
----------------
Thanks to Michael Brüstle of transputer.net for assistance with details of the T800, and for finding problems with
timer queue addresses - and for maintaining a superb archive.


