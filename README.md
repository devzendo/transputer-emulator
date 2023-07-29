# transputer-emulator

## What is this?

This is a portable, open source emulator of the 32-bit Inmos T414/T800/T801/T805 Transputer family, and a host/file
I/O Server that interfaces it to a host OS, providing boot/debug/IO facilities.

It runs on Apple macOS (From El Capitan to Catalina), Windows 10, CentOS 7.6, Ubuntu 16.04/18.04 and Raspbian Stretch.

It is part of the [Parachute Project](https://devzendo.github.io/parachute).

It is written in C++14 - the Raspbian Stretch distribution provides Clang 3.5.0, which does not support
more recent C++ standards.


## Project Status
First release 0.0.1 Midsummer 2019 (13 June 2019) as part of Parachute 0.0.1.

Project started around 19/08/2005, with a long hiatus. Another hiatus from Sep 2021 to Dec 2022.
Another hiatus from Dec 2022 to Jul 2023.

## Roadmap
First release:
* Ported to macOS (El Capitan +), Linux (Ubuntu 16.04, Ubuntu 18.04, CentOS 7.6, Raspbian Stretch), Windows 10.
* A Cross Platform system that can run "Hello World" (via my NodeServer (custom protocol) implementation).

Second release (work in progress):
* Convert the NodeServer to be IServer compatible, implementing all frame types needed by "Hello World" and
  eForth. Similarly, convert "Hello World" to be IServer compatible.
* IServer is unfinished; it implements the following frame types:
  * Id (Version)
  * Exit
  * Open
  * Read
  * Write
  * Get Key
  * Poll Key
  * Close
* Note that the IServer's file handling does not yet prevent directory traversal vulnerabilities. Also line feed
  handling on Windows is broken. To be corrected.
* Converting older C code (that's not very portable) to C++14 (that hopefully is more portable).
* Upgrade macOS build from El Capitan to Catalina (and beyond).
* eForth requires the following facilities of IServer (it currently accesses a UART; these routines must be changed
  to use IServer protocol on Link 0):
  * URD: Read and wait (odd return system)
  * ?RX: Return input character and indication of whether there is any input available
  * TX!: Send output character
  * !IO: Initialise UART
  
Third release:
* Capable of running eForth.
* Note that the IServer's file handling does not yet prevent directory traversal vulnerabilities. Also line feed
  handling on Windows is broken. To be corrected.

Fourth release:
* More IServer implementation.
* Prevent directory-traversal vulnerabilities in IServer's file handling - limit operations to under its root directory.
* Correct IServer translation of line feeds when dealing with text files (on Windows).
* Complete Integer functionality.

Fifth release:
* Complete Floating Point functionality.
* Complete IServer implementation.

# Release Notes
0.0.2 Second Release (work in progress)
* Converting the NodeServer to be IServer compatible. Similarly, "Hello World".
* Release for macOS is upgraded from El Capitan to Mojave. Untested on Catalina.

0.0.1 First Release
* Versioning and build now controlled by Maven and CMake.
* Successfully runs hello2.asm !
* Renamed emulator binary from t800emul to temulate.
* Started adding the T801 instructions, from "Transputer Instruction Set - Appendix, Guy Harriman".
* Started adding the T801/T805 instructions from "Support for debugging/breakpointing in transputers" (INMOS
  Technical Note 61).
  Added the -j flag to enable 'j 0' breakpoints.
* Described current implementation/missing status in the above section.
* The T810 instructions from "The IMS T810 - A Preliminary Survey" are not implemented.
* Builds using Maven/CMake/CLion on macOS, Windows 10, CentOS 7, Ubuntu LTS, Raspbian Stretch.
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
  

# Remaining Work
....

## IServer
* IServer client library in tmasm assembler
* Iserver needs to support terminal I/O facilities (no echo key reads, sensing available readable keys) for eForth

## Functionality
* j and cj - resolve the operand into an address that would be jumped to, display it
* Emulate multiple Transputers on 1-N physical cores
* Link topology management
* Investigate Benes networks
* Develop debug interface
* Write debugger program
* Add memory-mapped frame buffer via SDL
* Add mouse interface for same

## Correctness
* Remove potential buffer overflows in cpu.cpp (what were these?)
* Refactoring: make use of isLegalMemory within memory.cpp
* Unfinished: resetch when given a link not a memory channel

## Build/Releases
* Upgrade to C++14

## Bugs
* None known.

## Example Code
* Rework example code with IServer protocol library.

## Documentation
* Write some!


# Completeness
Although the emulator recognises all T414/T800/T801/T805 instructions, not all are currently implemented.
The greatest omission is floating point - this is an integer T805 at the moment!

The proposed T810 instructions are not implemented.

Those that are not emit a diagnostic when encountered.

See the sections 'Implemented Instructions' and 'Unimplemented Instructions' below.

It is my intention to eventually implement the complete instruction set. My focus is on the common integer instructions,
those that support communications, process management, diagnostics, and then the floating point instructions.

## Implemented Instructions
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

## Unfinished Instructions
fpchkerr, fptesterr, resetch, taltwt.

## Unimplemented Instructions
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


# Using the Emulator
Full documentation to follow.. but in the meantime...

Basically, given a binary boot file, then in two terminal sessions:
1) iserver -ld -df bootfile.bin        # The -ld -df is 'debug mode'.
2) temulate -df -ld -t                 # Ditto. -t terminates on memory violation.

The emulator waits for its boot file, down link 0. It then runs it.
The iserver sends the boot file, then switches to handle its host I/O protocol down link 0.

Run iserver or temulate with -? or -h to get their command line summaries.

To run the 'hello world' client program:

In one terminal window...

$ cd IServer/client-examples/hello2
$ tmasm  -b hello2.bin -l hello2.lst hello2.asm
Pass 1: Creating model from 233 macro-expanded line(s)
End of Pass 1: Checking for unresolved forward references
Pass 2: Updating model with 0 pass 2 section(s)
End of Pass 2
Writing binary file hello2.bin
Start address 0x8000006F
End address 0x80000147
Writing listing file hello2.lst

$ iserver hello2.bin
(does not return)

In second terminal window...
$ temulate
$

The first terminal window should now show:
$ iserver hello2.bin
hello world
(still does not return)
Ctrl-C <<- you'll have to interrupt it.
$


# Source directory structure
The source is split into the following directories:

Shared - utility code that is common to many parts of the system.

IServer - the client and server portions of the file/host I/O server and its protocol
definition. The server runs on your host computer (i.e. under Windows, Linux,
macOS etc.). The client runs under Transputer emulation as part of your
application; it's an assembly language include file.

Emulator - the T414/T800/T801/T805 emulator.


# Building and Installing
The distribution currently builds under the following systems:
* Apple macOS 'Mojave' 10.14 (untested on more recent versions)
* Windows 10 64-bit (untested on earlier versions e.g. XP, 7, 8, 8.1)
* CentOS Linux 7.6 Intel x86-64
* Raspbian Stretch
* Ubuntu Linux 16.04.4 LTS Intel x86-64
* Ubuntu Linux 18.04.2 LTS Intel x86-64

C++ Dependencies:
* Google Test and Google Mock
* gsl-lite (C++ Guidelines Support Library from https://github.com/gsl-lite/gsl-lite#as-cmake-package)
* .. all taken care of by CMake External Project.

Prerequisites:
- All Operating Systems:
  - Git (to clone the transputer-emulator repository, and for CMake to download GoogleTest). Command line tool git needs
    to be on the PATH.
  - Python (prefer 3.x but 2.x is fine; required by the GoogleTest build). Command line interpreter python needs to be
    on the PATH.
  - Java 8 JDK (for Maven).
  - Apache Maven. I use 3.6.0. (You can build without it, it's just doing some preprocessing, running cmake in various
    stages, and is used for packaging and overall build control. It's just much easier with it.) Command line tool mvn
    needs to be on the PATH.
  - CMake. I use 3.10.3. Command line tool cmake needs to be on the PATH.
  - If you want to build the client-examples programs, you'll need the
    <a href="https://bitbucket.org/devzendo/transputer-macro-assembler">DevZendo.org Transputer Macro Assembler</a>
    installed and on your PATH.
- macOS 'Mojave' 10.14.6:
  - CMake 3.15.5 (from MacPorts) and 3.14.3 (from CLion)
  - GNU Make. I use 4.2.1.
  - clang (e.g. via XCode Developer tools, or MacPorts). Apple LLVM version 11.0.0 (clang-1100.0.33.8) on
  - Untested on more recent versions of macOS
  - If you see messages like this in a build failure:
    "Ignoring CMAKE_OSX_SYSROOT value:
    /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.15.sdk
    because the directory does not exist."
    Then this is due to recent XCode removing the version directory - as root, do:
    cd /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs
    ln -s MacOSX.sdk MacOSX10.15.sdk
- Windows 10 Home:
  - Microsoft Visual Studio Build Tools 2017, v 15.9.28307.423
    (cl 19.16.27027.1 for x64) Note this doesn't need the full Visual Studio, these are just the command line tools.
    The command line tools must be set up in your command line environment. In my shell, I run:
    "C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x86_amd64
    prior to running other commands.
  - CMake is installed in C:\Program Files\CMake
  - Maven is installed in C:\Program Files\apache-maven-3.6.0
  - (these locations are noted in the pom.xml).
  - Note if you're using CLion, set your Visual Studio Toolchain architecture to amd64. Otherwise you'll get errors
    saying your compiler can't create a simple executable.
- Ubuntu 16.04:
  - Clang etc.:
    apt-get install build-essential clang-6.0 make clang
- Ubuntu 18.04:
  - Clang etc.:
    apt-get install build-essential clang-7 make clang
- CentOS 7.6.1810:
  - Clang/LLVM 7:
    yum install centos-release-scl-rh
    yum --enablerepo=centos-sclo-rh-testing install devtoolset-7 devtoolset-7-llvm
    (Clang 5.0.1)
- Raspbian Stretch:
  - Clang 3.5.0

The typical install location is:
- macOS/Linux: /opt/parachute
- Windows: C:\parachute

## Building
To build, cd to the top level directory (where this README.md is) and do:
On windows: vcvarsall.bat as shown above
mvn clean compile -P build

This will:
 * download all dependencies and plugins (quite a few of these), 
 * create the version number header (the version is controlled by the maven pom.xml)
 * creates the shared library code that contains the project version, in the target/classes directory, then
 * run this for you: 
   * cd cmake-build-debug; cmake .. (ie regenerate the cmake cache)
   * cmake --build cmake-build-debug --target all -- -j 4

This build will build the entire system: T800 emulator and iserver, client libraries, etc. 

This doesn't install it on your system - see below.

## Cleaning the Build Tree
To clean:
mvn clean
This effectively does:
rm -rf cmake-build-debug

## Installing the Built Code
To install into the default install location, you'll need to have permission to create it and
write files there. 
e.g. on macOS/Linux:
$ sudo mkdir /opt/parachute
Password: <enter your password, assuming you have sudo rights>
$ sudo chown myuser:myuser /opt/parachute # e.g. Linux
$ sudo chown myuser:staff /opt/parachute  # e.g. macOS

e.g. on Windows:
In File Explorer, create C:\parachute and set it writable by your user account, however you do this.

Then to copy the built software there:
mvn -P local-install prepare-package 

This installation location is defined in the operating-system-specific profile sections of the pom.xml.



# Acknowledgements
This project would not have been possible without the hard work and inspiration of many individuals.

Notably, thanks to:

Michael Brüstle of transputer.net for assistance with details of the T800, and for finding problems with
timer queue addresses - and for maintaining a superb archive.

Dr. Barry Cook, formerly of Keele University, for starting my interest in the transputer, and for my final year
undergraduate Computer Science project, also a transputer emulator (with different goals).

The transputer designers, and all at Inmos/ST who developed it.

Julian Highfield for his T414 emulator.

Yury Shevchuk and Roman Pozlevich for the gcc-t800 port, ttools and libxputer packages.  These were used throughout
an earlier version of the project, for bootstrap code, object and executable file format and loader.


# Bibliography
[CWG] "Transputer Instruction Set - A Compiler Writer's Guide"
 http://www.transputer.net/iset/pdf/tis-acwg.pdf

"Transputer Instruction Set - A Compiler Writer's Guide - Errata"
 http://www.transputer.net/iset/errata/tis-acwg-errata.txt

"Transputer Instruction Set - Appendix, Guy Harriman"
 http://www.transputer.net/iset/appendix/appendix.pdf

"Support for debugging/breakpointing in transputers" (INMOS Technical Note 61)
  http://www.transputer.net/tn/61/tn61.pdf

[TTH] "The Transputer Handbook", Ian Graham, Gavin King
  http://www.transputer.net/iset/isbn-013929134-2/tthb.pdf

"IMS T805 32-bit floating point transputer"
 http://www.transputer.net/ibooks/dsheets/t805.pdf

"Transputer Versions - Mike's Technical Note 2, Michael Brüstle"
 http://www.transputer.net/tn/m02/tdiff.pdf

"The IMS T810 - A Preliminary Survey, Guy Harriman"
  http://www.transputer.net/fbooks/t810pre/t810pre.pdf

"Transputer Development System, second edition"
  http://transputer.net/prog/72-trn-011-01/tds2nd.pdf


# License, Copyright & Contact info
This code is released under the Apache 2.0 License: http://www.apache.org/licenses/LICENSE-2.0.html.

(C) 2005-2023 Matt J. Gumbley

matt.gumbley@devzendo.org

Mastodon: @M0CUV@mastodon.radio

Twitter: (abandoned) @mattgumbley @devzendo

http://devzendo.github.io/parachute

