transputer-emulator
===================
This is an emulator of the 32-bit Inmos T414/T800/T801/T805 Transputer family, and a Node Server that interfaces it
to a host OS, providing boot/debug/IO facilities.

It is part of the [Parachute Project](https://devzendo.github.io/parachute).

(C) 2005-2019 Matt J. Gumbley
matt.gumbley@devzendo.org
@mattgumbley @devzendo
http://devzendo.github.io/parachute

Status
------
In progress, January 2019. Modernising, building on modern OSX/Linux/Windows.

Release Notes
-------------
0.01 (ongoing work for the first release)
* Versioning and build now controlled by Maven and CMake.
* Successfully runs hello2.asm !
* Renamed emulator binary from t800emul to temulate.
* Started adding the T801 instructions, from "Transputer Instruction Set - Appendix, Guy Harriman".
* Started adding the T801/T805 instructions from "Support for debugging/breakpointing in transputers" (INMOS
  Technical Note 61).
  Added the -j flag to enable 'j 0' breakpoints.
* Described current implementation/missing status in the above section.
* The T810 instructions from "The IMS T810 - A Preliminary Survey" are not implemented.
* Builds using CMake/CLion on OSX.
* Added Boot-from-ROM, fixed Wdesc bug after boot from link.
* Fixes: xword, call, j & scheduling (with assistance from Michael Brüstle), locations of TPtrLoc1, TPtrLoc0.
* Monitor: db (renamed from da), dw improvements, added w (workspace display), added b/b+b-/b? (breakpoints),
  added s (show all state).
* Adopted the Apache License v2.
* Since the assembler understands offset addressing, the manual building of offset operands has been changed from:
  j 'label - _XX1'
  _XX1:
  to:
  j label
  

Remaining Work
--------------
....

Node Server
===========
* Replace NodeServer with an iserver/afserver-protocol compatible version
* Node Server client library in tmasm assembler
* Node server needs to support terminal I/O facilities (no echo key reads, sensing available readable keys) for eForth

Functionality
=============
* j and cj - resolve the operand into an address that would be jumped to, display it
* Emulate multiple Transputers on 1-N physical cores
* Link topology management
* Investigate Benes networks
* Develop debug interface
* Write debugger program
* Add memory-mapped frame buffer via SDL
* Add mouse interface for same

Correctness
===========
* Remove potential buffer overflows in cpu.cpp (what were these?)
* Refactoring: make use of isLegalMemory within memory.cpp
* Unfinished: resetch when given a link not a memory channel

Build/Releases
==============
* Complete conversion of build scripts to cmake, remove GNU make files
* Build on Linuxes
* Build on Windows
* Fix all compiler warnings (some tautologous comparisons remain)
* Upgrade to C++11 or more recent

Bugs
====
* (possibly obviated by CMake) why does 'make test' in Shared says it fail to link, yet actually does, and works?

Example Code
============
* Rework example code with iserver protocol library.

Documentation
=============
* Write some!

Completeness
------------
Although the emulator recognises all T414/T800/T801/T805 instructions, not all are currently implemented.
The greatest omission is floating point - this is an integer T805 at the moment!

The proposed T810 instructions are not implemented.

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

T805:
break, clrj0break, lddevid, ldmemstartval, pop, setj0break, testj0break.

Nonstandard emulator:
emuquery, marker, terminate, toggledisasm, togglemonitor

Unfinished Instructions
=======================
fpchkerr, fptesterr, resetch, taltwt.

Unimplemented Instructions
==========================
fpuexpdec32, fpadd, fpb32tor64, fpchki32, fpdiv, fpdup, fpeq, fpgt,
fpi32tor32, fpi32tor64, fpint, fpldnladddb, fpldnladdsn, fpldnldb, fpldnldbi,
fpldnlmuldb, fpldnlmulsn, fpldnlsn, fpldnlsni, fpldzerodb, fpldzerosn, fpmul,
fpnan, fpnotfinite, fpordered, fpremfirst, fpremstep, fprev, fprtoi32, fpstnldb,
fpstnli32, fpstnlsn, fpsub, fpuabs, fpuchki64, fpudivby2, fpuexpinc32, fpumulby2,
fpunoround, fpur32tor64, fpur64tor32, fpurm, fpurn, fpurp, fpurz, fpusqrtfirst,
fpusqrtlast, fpusqrtstep, norm, testpranal.

32-bit Transputers: fmul.

T414: cflerr, ldinf, postnormsn, roundsn, unpacksn.

T800: crcbyte, crcword, move2dall, move2dinit, move2dnonzero, move2dzero.

T801:
start, testhardchan, testldd, testlde, testlds, teststd, testste, teststs.

T805:
timerenableh, timerenablel, timerdisableh, timerdisablel.

T810: checkaddr, delay, dislinkinth, dislinkintl, distimesl, enlinkinth, enlinkintl,
entimesl, fpmacc, fpxprod, ldhw, macc, pause, sthw, xprod


Using the Emulator
------------------
Read the manual in Documentation/Manual (Requires OpenOffice/LibreOffice).
Basically, given a binary boot file, then in two sessions:
1) nodeserver -ld -df bootfile.bin     # The -ld -df is 'debug mode'.
2) temulate -df -ld -t                 # Ditto. -t terminates on memory violation.
The emulator waits for its boot file, down link 0. It then runs it. The nodeserver sends the boot file, then switches
to handle its host I/O protocol down link 0.
Run nodeserver or temulate with -? or -h to get their command line summaries.

Directory structure
-------------------
The source is split into the following directories:

Shared - utility code that is common to many parts of the system.

NodeServer - the client and server portions of the node server and its protocol
definition. The server runs on your host computer (i.e. under Windows, Linux,
Mac OS X etc.). The client runs under Transputer emulation as part of your
application; it's an assembly language include file.

Emulator - the T414/T800/T801/T805 emulator.


Building and Installing
-----------------------
The distribution currently builds under the following systems:
* Mac OSX 'El Capitan' 10.11

Later I intend to provide buils for:
* Ubuntu Linux 16.04.4 LTS Intel x86-64
* CentOS Linux 7.4 Intel x86-64
* Windows 10

Prerequisites:
- All:
  - <a href="https://bitbucket.org/devzendo/transputer-macro-assembler">DevZendo.org Transputer Macro Assembler</a>
    installed and on your PATH.
  - GNU Make.
  - Apache Maven. (You can build without it, it's just doing some preprocessing, running cmake in various stages, and
    is used for packaging.)
- OSX: clang (e.g. via XCode Developer tools, or MacPorts), cmake 3.10
- Ubuntu Linux: build-essential (=> gcc) [DOES NOT BUILD HERE YET]
- CentOS: gcc [DOES NOT BUILD HERE YET]
- Windows: [DOES NOT BUILD HERE YET] (future work, but you'll need a UNIXy toolchain)


The install location is /opt/parachute. Changing this would entail changes to
Makefiles in the above hierarchy.

To build, cd to the top level directory (where this README.md is) and do:
mvn compile

This creates the shared library code that contains the project version, in
the target/classes directory, then does: 
cd cmake-build-debug; cmake .. (ie regenerate the cmake cache)
cmake --build cmake-build-debug --target all -- -j 4

To clean:
mvn clean
This effectively does:
rm -rf cmake-build-debug

To install: (this needs reworking to use cmake - the Makefiles are to be replaced with cmake)

make install

During the install, you will be prompted for a password. This is since the files
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


Bibliography
------------
[CWG] "Transputer Instruction Set - A Compiler Writer's Guide"
 http://www.transputer.net/iset/pdf/tis-acwg.pdf

"Transputer Instruction Set - A Compiler Writer's Guide - Errata"
 http://www.transputer.net/iset/errata/tis-acwg-errata.txt

"Transputer Instruction Set - Appendix, Guy Harriman"
 http://www.transputer.net/iset/appendix/appendix.pdf

"Support for debugging/breakpointing in transputers" (INMOS Technical Note 61)
  http://www.transputer.net/tn/61/tn61.pdf

[TTH] "The Transputer Handbook", Ian Graham, Gavin King
 Not available online.

"IMS T805 32-bit floating point transputer"
 http://www.transputer.net/ibooks/dsheets/t805.pdf

"Transputer Versions - Mike's Technical Note 2, Michael Brüstle"
 http://www.transputer.net/tn/m02/tdiff.pdf

"The IMS T810 - A Preliminary Survey, Guy Harriman"
  http://www.transputer.net/fbooks/t810pre/t810pre.pdf