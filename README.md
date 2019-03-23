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
In progress, March 2019. Modernising, building on modern OSX/Linux/Windows.

* Building using Maven/CMake/Microsoft Visual Studio Command line tools on Windows 10 64-bit, and porting POSIX calls
  to Win32 API.
* Building using Maven/CMake/Clang on CentOS 7 and Raspbian Stretch.
* Converting older C code (that's not very portable) to C++11(+) (that hopefully is).

Roadmap
-------
First release:
* Ported to Mac OSX (El Capitan +), Linux (Ubuntu, CentOS 7, Raspbian Stretch), Windows 10.
* A Cross Platform system that can run "Hello World" (via my NodeServer implementation).

Second release:
* Convert the NodeServer to be iServer compatible. Similarly, "Hello World".

Third release:
* Capable of running eForth.

Fourth release: 
* Complete Integer functionality.

Fifth release:
* Complete Floating Point functionality.

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
* Builds using Maven/CMake/CLion on OSX.
* Added Boot-from-ROM, fixed Wdesc bug after boot from link.
* Fixes: xword, call, j & scheduling (with assistance from Michael Brüstle), locations of TPtrLoc1, TPtrLoc0.
  csngl and xdble: correct detection of sign of Areg
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
* Build on Linuxes
* Upgrade to C++11 or more recent

Bugs
====
* None known.

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
Full documentation to follow.. but in the meantime...

Basically, given a binary boot file, then in two terminal sessions:
1) nodeserver -ld -df bootfile.bin     # The -ld -df is 'debug mode'.
2) temulate -df -ld -t                 # Ditto. -t terminates on memory violation.

The emulator waits for its boot file, down link 0. It then runs it.
The nodeserver sends the boot file, then switches to handle its host I/O protocol down link 0.

Run nodeserver or temulate with -? or -h to get their command line summaries.

To run the 'hello world' client program:

In one terminal window...

$ cd NodeServer/client-examples/hello2
$ tmasm  -b hello2.bin -l hello2.lst hello2.asm
Pass 1: Creating model from 233 macro-expanded line(s)
End of Pass 1: Checking for unresolved forward references
Pass 2: Updating model with 0 pass 2 section(s)
End of Pass 2
Writing binary file hello2.bin
Start address 0x8000006F
End address 0x80000147
Writing listing file hello2.lst

$ nodeserver hello2.bin
(does not return)

In second terminal window...
$ temulate
$

The first terminal window should now show:
$ nodeserver hello2.bin
hello world
(still does not return)
Ctrl-C <<- you'll have to interrupt it.
$


Source directory structure
--------------------------
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
* Mac OSX 'El Capitan' 10.11 (untested on more recent versions)
* Windows 7 (untested on more recent versions especially 10)
* CentOS Linux 7.4 Intel x86-64

* Raspbian Stretch (ongoing)

Later I intend to provide builds for:
* Ubuntu Linux 16.04.4 LTS Intel x86-64

Prerequisites:
- All Operating Systems:
  - Mercurial (to download the source). I use SourceTree on OSX and Windows, TortoiseHg on Windows, and hg on Linux.
  - Git (for CMake to download GoogleTest). Command line tool needs to be on the PATH.
  - Python (2.x or 3.x is fine) (required by the GoogleTest build)
  - Java 8 JDK (for Maven).
  - Apache Maven. I use 3.6.0. (You can build without it, it's just doing some preprocessing, running cmake in various
    stages, and is used for packaging and overall build control.)
  - CMake. I use 3.10.3.
  - If you want to build the client-examples programs, you'll need the
    <a href="https://bitbucket.org/devzendo/transputer-macro-assembler">DevZendo.org Transputer Macro Assembler</a>
    installed and on your PATH.
- OSX:
  - GNU Make. I use 4.2.1.
  - clang (e.g. via XCode Developer tools, or MacPorts). I use Apple LLVM version 8.0.0 (clang-800.0.42.1) on
    Mac OSX 'El Capitan' 10.11.6
- Windows:
  - Microsoft Visual Studio Build Tools 2017, v 15.9.xxx
  - CMake is installed in C:\Program Files\CMake
  - Maven is installed in C:\Program Files\apache-maven-3.6.0
  - (these locations are noted in the pom.xml).
- Ubuntu Linux: build-essential (=> gcc) [DOES NOT BUILD HERE YET]
- CentOS 7:
  - Clang/LLVM 7:
    yum install centos-release-scl-rh
    yum --enablerepo=centos-sclo-rh-testing install devtoolset-7 devtoolset-7-llvm
- Raspbian Strecth:
  - Clang or gcc (latest versions - TODO note the versions here)

The typical install location is:
- OSX/Linux: /opt/parachute
- Windows: C:\parachute

Building
========
To build, cd to the top level directory (where this README.md is) and do:
mvn clean compile

This creates the shared library code that contains the project version, in
the target/classes directory, then does: 
cd cmake-build-debug; cmake .. (ie regenerate the cmake cache)
cmake --build cmake-build-debug --target all -- -j 4

This build will build the entire system: T800 emulator and node
server, client libraries, etc. This doesn't install it on your system - see below.

Cleaning the Build Tree
=======================
To clean:
mvn clean
This effectively does:
rm -rf cmake-build-debug

Installing the Built Code
=========================
To install into the default install location, you'll need to have permission to create it and
write files there. 
e.g. on OSX/Linux: 
$ sudo mkdir /opt/parachute
Password: <enter your password, assuming you have sudo rights>
$ sudo chown myuser:myuser /opt/parachute # e.g. Linux
$ sudo chown myuser:staff /opt/parachute  # e.g. OSX

e.g. on Windows:
In File Explorer, create C:\parachute and set it writable by your user account, however you do this.

Then to copy the built software there:
mvn prepare-package 

This installation location is defined in the operating-system-specific profile sections of the pom.xml.


License
-------
This code is released under the Apache 2.0 License: http://www.apache.org/licenses/LICENSE-2.0.html.
(C) 2005-2019 Matt Gumbley, DevZendo.org


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
