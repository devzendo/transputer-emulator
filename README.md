# transputer-emulator

## What is this?
This is a portable, open source emulator of the 32-bit Inmos T414/T800/T801/T805 Transputer family, and a host/file
I/O Server that interfaces it to a host OS, providing boot/debug/IO facilities.

It runs on Apple macOS Catalina, Windows 10, Ubuntu 24.02, Linux Mint 21.3, and Raspberry Pi Debian 12.
An embedded version of the emulator component is also available for the Raspberry Pi Pico 1.
I do not have Mac hardware to support porting beyond Catalina; specifically no M1/M2/Mx.
Due to lack of compatible TPM hardware, I cannot build on Windows 11 or later.

It is part of the [Parachute Project](https://devzendo.github.io/parachute).

It is written in C++14. I do not intend to move beyond this C++ version.

It used to build on Raspbian Stretch, the oldest OS I had when I started automated builds. This had Clang 3.5.0, which
limited the C++ standard I could support.


## Project Status
Actively in development.

Currently working on compatibility with the aid of Mike Brüstle's Transputer Validation Suite.

Also adding debugging facilities to aid the completion of the eForth Transputer port, and adding a build
for the Raspberry Pi Pico.

Attempting to run other typical Transputer software such as the Inmos occam and C compilers, and the port of Minix. 

First release 0.0.1 Midsummer 2019 (13 June 2019) as part of Parachute 0.0.1.

Project started around 19/08/2005, with a long hiatus.
Another hiatus from Sep 2021 to Dec 2022.
Another hiatus from Dec 2022 to Jul 2023.
Another hiatus from Dec 2023 to Aug 2024.


## Roadmap
First release:
* A Cross Platform system that can run "Hello World" (via my NodeServer (custom protocol) implementation).

Second release (work in progress):
* Convert the NodeServer to be IServer compatible, implementing all frame types needed by "Hello World" and
  eForth. Similarly, convert "Hello World" and eForth to be IServer compatible.
* IServer is unfinished; it implements the following frame types:
  * Id (Version)
  * Exit
  * Open
  * Read
  * Write
  * Puts (put string with newline to a stream)
  * Get Key
  * Poll Key
  * Close
  * Command Line
* It implements the following "extended" frame types, not part of the original IServer, for performance purposes:
  * Put Char (to the console stream)
* Note that the IServer's file handling does not yet prevent directory traversal vulnerabilities.
* Converting older C code (that's not very portable) to C++14 (that hopefully is more portable).
* eForth requires the following facilities of IServer (it originally accessed a UART; these routines have been
  changed to use IServer protocol on Link 0):
  * URD: Read and wait (odd return system)
  * ?RX: Return input character and indication of whether there is any input available
  * TX!: Send output character
  * !IO: Initialise UART
* Allow validation using Mike's TVS and pass tests for all implemented instructions
* Add the start of a build for the Raspberry Pi Pico - that can run Hello World booted from a link; that has a single
  core, and single link over CDC-USB Serial. The memory/flash use is quantified.
  
Third release:
* Capable of running eForth.
* Note that the IServer's file handling does not yet prevent directory traversal vulnerabilities.

Fourth release:
* More IServer implementation.
* Prevent directory-traversal vulnerabilities in IServer's file handling - limit operations to under its root directory.
* Complete Integer functionality.

Fifth release:
* Complete Floating Point functionality.
* Complete IServer implementation.


# Release Notes
0.0.2 Second Release (work in progress)
* Converting the NodeServer to be IServer compatible. Similarly, "Hello World"
  (see IServer/client-examples/hello-world-iserver).
* Release for macOS is upgraded from El Capitan to Catalina.
* Release for Intel Linux is now done only on Ubuntu 24.04.
* Release for Raspberry Pi is now done only on Debian 12.
* Release for Windows 10 is now done on Windows 10 22H2.
* Removed versions for older OSs (macOS El Capitan, Ubuntu 16.04, 18.04, CentOS 7.6, Raspbian Stretch).
* IServer -df (full debug) now enables all parts of the debug output.
* Successfully runs hello.asm !
* IServer now supports FPuts and the extended PutChar frame (used by eForth); it 
  also supports CommandLine, for use by (e.g.) the Inmos compilers. Any IServer
  arguments that are not understood by IServer can be passed to the transputer on
  request.
* Added a "Hello World" example that uses the macro assembler's primary bootstrap
  include file. (see IServer/client-examples/hello-world-secondary-iserver)
* Added a "Hello World" example that can boot from ROM.
  (see IServer/client-examples/hello-world-iserver-rom)
* Bugfix: A loaded ROM's memory is now initialised/destroyed correctly.
* Emulator now allows a list of symbols to be loaded; these are displayed when
  disassembling or using the monitor; also enhancements for debugging eForth.
* The emulator's monitor now accepts a 't' command which toggles the display of
  disassembly and memory read/write.
* The emulator can run tests from Mike Brüstle's Transputer Validation Suite.
* Corrected several instruction implementations with guidance from TVS: add,
  adc, bitrevnbits, bitrevword, csngl, cword, div, ladd, ldiff, ldiv, lmul,
  lshl, lshr, lsub, lsum, mul, rem, shl, shr, sub, wcnt, xdble, xword.
* Implemented the instructions from "Transputer Instruction Set - Appendix".
* Add a build of the emulator for the Raspberry Pi Pico 1.
* Bugfix: protocol handler - open file - was inadvertantly broken on some
  platforms.

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
* Builds using Maven/CMake/CLion on macOS (El Capitan +), Linux (Ubuntu 16.04, Ubuntu 18.04, CentOS 7.6, Raspbian
  Stretch), Windows 10.
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
* Develop debug interface
* Write debugger program
* Add memory-mapped frame buffer via SDL
* Add mouse interface for same
* Investigate Benes networks

## Correctness
* Remove potential buffer overflows in cpu.cpp (what were these?)
* Refactoring: make use of isLegalMemory within memory.cpp
* Unfinished: resetch when given a link not a memory channel

## Build/Releases
* The debug/release configuration switch does not appear to be doing anything other than setting an appropriate build
  directory.

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

Those that are not implemented emit a diagnostic when encountered.

See the sections 'Implemented Instructions' and 'Unimplemented Instructions' below.

It is my intention to eventually implement the complete instruction set, such
that it completely passes the Transputer Validation Suite. My focus is on the
common integer instructions, those that support communications, process
management, diagnostics, and then the floating point instructions.

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

From the Transputer Instruction Set - Appendix:
start, testlds, teststs.

T414:
cflerr

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

From the Transputer Instruction Set - Appendix:
testhardchan, testldd, testlde, teststd, testste, 

32-bit Transputers:
fmul.

T414:
ldinf, postnormsn, roundsn, unpacksn.

T800:
crcbyte, crcword, move2dall, move2dinit, move2dnonzero, move2dzero.

T805:
timerenableh, timerenablel, timerdisableh, timerdisablel.

T810: checkaddr, delay, dislinkinth, dislinkintl, distimesl, enlinkinth, enlinkintl,
entimesl, fpmacc, fpxprod, ldhw, macc, pause, sthw, xprod

## Instructions required for eForth
ldc, ldpi, dup, adc, rev, ldnl, gcall
stl, ldl, wcnt, gajw, mint, clrj0break,
timerdisableh, timerdisablel, toggledisasm,
outbyte, outword, ldlp, in, terminate, cj, sb,
and, eqc, lb, not, or, xor, lsum.

## Transputer Validation Suite conformance
tests: 54, ok: 46, fail: 8

```
        adc PASS
        add PASS
        alt PASS
        and PASS
       bcnt PASS
     bitcnt PASS
bitrevnbits PASS
 bitrevword PASS
       bsub PASS
      ccnt1 PASS
     cflerr PASS
    crcbyte unimplemented FAIL
    crcword unimplemented FAIL
      csngl PASS
      csub0 PASS
      cword PASS
       diff PASS
        div PASS
        dup PASS
        eqc PASS
       fmul unimplemented FAIL
         gt PASS
       ladd PASS
        ldc PASS
      ldiff PASS
      ldinf unimplemented FAIL
       ldiv PASS
       lmul PASS
       lshl PASS
       lshr PASS
       lsub PASS
       lsum PASS
       mint PASS
        mul PASS
       norm unimplemented FAIL
        not PASS
         or PASS
 postnormsn unimplemented FAIL
       prod PASS
        rem PASS
        rev PASS
    roundsn unimplemented FAIL
        shl PASS
        shr PASS
        sub PASS
        sum PASS
       talt PASS
   unpacksn unimplemented FAIL
       wcnt PASS
       wsub PASS
     wsubdb PASS
      xdble PASS
        xor PASS
      xword PASS
```


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

$ cd IServer/client-examples/hello-world-iserver
$ tmasm -b hello.bin -l hello.lst hello.asm
Pass 1: Creating model from 245 macro-expanded/included line(s)
End of Pass 1: Checking for unresolved forward references
Pass 2: Updating model with 0 pass 2 section(s)
End of Pass 2
Writing binary file hello.bin
Start address 0x8000006F
End address 0x80000162
Writing listing file hello.lst

$ iserver hello.bin
(does not return)

In second terminal window...
$ temulate
$ (automatically returns, as the 'terminate' instruction is the last executed in hello.asm)

The first terminal window should now show:
$ iserver hello.bin
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
* Apple macOS - Intel only
  * 'Catalina' 10.15 
  * (untested on more recent versions)
* Linux
  * Linux Mint 21.3 Intel x86-64
  * Ubuntu Linux 24.04 LTS Intel x86-64
  * Raspberry Pi Debian 12
* Microsoft Windows
  * Windows 10 22H2
  * (untested on earlier versions e.g. XP, 7, 8, 8.1)
  * Due to lack of compatible TPM hardware, I cannot build on Windows 11 or later.
* Embedded microcontrollers
  * Raspberry Pi Pico (cross-compiled on Ubuntu Linux 24.04)

The build is controlled by CMake, with a Maven wrapper around this to work with my CI server, and to make the main
build commands a little easier to use (standard mvn lifecycle commands vs CMake oddities). Maven is just doing some
preprocessing (the version number of the project is set in the pom.xml; mvn extracts this and embeds it in the C++ code),
running cmake in various stages, and is used for packaging and overall build control. It's just much easier with it..

You can ignore the Maven side completely, you'll just have to use arcane commands.

Both types of build are shown below.

C++ Dependencies:
* These are all downloaded and built by CMake External Projects:
* Google Test and Google Mock
* gsl-lite (C++ Guidelines Support Library from https://github.com/gsl-lite/gsl-lite#as-cmake-package)

Prerequisites:
- All Operating Systems:
  - Git (to clone the transputer-emulator repository, and for CMake to download GoogleTest). Command line tool git needs
    to be on the PATH.
  - Python (prefer 3.x but 2.x is fine; required by the GoogleTest build). Command line interpreter python needs to be
    on the PATH.
  - Java 17 JDK (for Maven).
  - Optional: Apache Maven. I use 3.9.9. Command line tool mvn needs to be on the PATH. It's recommended to download
    the latest binary release from https://maven.apache.org/download.cgi and unzip it, placing its bin directory on the PATH.
  - CMake. I use 3.10.3+ .. 3.25.1. Command line tool cmake needs to be on the PATH. 
  - If you want to build the client-examples programs, you'll need the
    <a href="https://bitbucket.org/devzendo/transputer-macro-assembler">DevZendo.org Transputer Macro Assembler</a>
    installed and on your PATH.
  - The TMPDIR environment variable must be set to something on all POSIX systems, or some tests will fail.
- macOS 'Catalina' 10.15:
  - CMake 3.29.5 (from MacPorts)
  - GNU Make 3.81 (from XCode command line tools)
  - clang (e.g. via XCode Developer tools). Apple clang version 12.0.0 (clang-1200.0.32.29)
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
  - Maven is installed in C:\Program Files\apache-maven-3.9.9
  - (these locations are noted in the pom.xml).
  - Note if you're using CLion, set your Visual Studio Toolchain architecture to amd64. Otherwise you'll get errors
    saying your compiler can't create a simple executable.
- Ubuntu 24.04, Linux Mint 21.3:
  - G++ 11 etc:
    apt-get install build-essential g++-11 make cmake
  - CMake: install it via apt, and create symlinks /opt/cmake/bin/cmake -> /usr/bin/cmake and
    /opt/cmake/bin/ctest -> /usr/bin/ctest .
- Debian 12 Raspberry Pi:
  - G++ 11 etc:
    apt-get install build-essential g++-11 make cmake
 
- Raspberry Pi Pico (build on Ubuntu 24.04)
  - The Pico SDK and toolchain:
    `apt install gcc-arm-none-eabi libnewlib-arm-none-eabi build-essential`
    The Pico SDK should be cloned from https://github.com/raspberrypi/pico-sdk
    Set the `PICO_SDK_PATH` environment variable to point to this copy of the Pico SDK.
    The Pico Extras should be cloned into the directory alongside the SDK from https://github.com/raspberrypi/pico-extras
    Set the `PICO_EXTRAS_PATH` environment variable to point to this copy of the Pico Extras.
    i.e. the pico-sdk and pico-extras clones are siblings.
    CMake can be installed from apt, no need for a manual install.

The typical install location is:
- macOS/Linux: /opt/parachute
- Windows: C:\parachute
- Raspberry Pi Pico: temulate.uf2 - copy it to your Pico with BOOTSEL held down

## Building
To build, cd to the top level directory (where this README.md is) and do:

On windows: `vcvarsall.bat` as shown above.

Then for desktop builds (e.g. macOS - adjust for Windows):

Using CMake directly:
setup:
  `mkdir -p target/classes`
  `cp src/main/resources/* target/classes`
  edit `target/classes/version.cpp` and change `${project.version}` to the version shown near the top of pom.xml
generate:
  `mkdir cmake-build-debug`
  `cd cmake-build-debug`
  `cmake -G "Unix Makefiles" -D NOCROSS=true /path/to/transputer-emulator`
  `cd ..`
compile:
  `cmake --build cmake-build-debug --target all`
test:
  `ctest --test-action Test --parallel 8 --verbose`
  
  
Using the Maven wrapper:
setup/generate/compile:
  `mvn clean compile -P build`
test:
  `mvn test -P build`
package:
  `mvn package -P build`


NOTE: if you have a test failure from testfilesystem, set the TMPDIR environment variable to /tmp.

For Raspberry Pi Pico:
  `mvn -DCROSS=PICO clean compile -P build`

The Maven build will:
 * download all dependencies and plugins (quite a few of these), 
 * create the version number header (the version is controlled by the maven pom.xml)
 * creates the shared library code that contains the project version, in the target/classes directory, then
 * run this for you: 
   * cd cmake-build-debug; cmake .. (ie regenerate the cmake cache)
   * cmake --build cmake-build-debug --target all -- -j 4

This build will build the entire system: T800 emulator and iserver, client
libraries, etc.  (Only the emulator is built for Rasperry Pi Pico.)

This doesn't install it on your system - see below.

## Cleaning the Build Tree
To clean:
  `mvn clean`

## Installing the Built Code
To install into the default install location, you'll need to have permission to create it and
write files there. 
e.g. on macOS/Linux:
```
$ sudo mkdir /opt/parachute
Password: <enter your password, assuming you have sudo rights>
$ `sudo chown myuser:myuser /opt/parachute # e.g. Linux
$ sudo chown myuser:staff /opt/parachute  # e.g. macOS
```

e.g. on Windows:
In File Explorer, create C:\parachute and set it writable by your user account, however you do this.

Then to copy the built software there:
  `mvn -P local-install prepare-package`

This installation location is defined in the operating-system-specific profile sections of the pom.xml.



# Acknowledgements
This project would not have been possible without the hard work and inspiration
of many individuals.

Notably, thanks to:

Michael Brüstle of transputer.net for invaluable assistance with details of the
T800, idiosyncrasies of its operation, aspects of undefined behaviour, for
finding problems with timer queue addresses - and for maintaining a superb
archive. Also for granting access to the Transputer Validation Suite (which is
not for redistribution).

Dr. Barry Cook, formerly of Keele University, for starting my interest in the
transputer, and for my final year undergraduate Computer Science project, also a
transputer emulator (with different goals).

The transputer designers, and all at Inmos/ST who developed it.

Authors of other emulators:

Julian Highfield for his T414 emulator which may be found at 
https://web.archive.org/web/20130515034826/http://spirit.lboro.ac.uk/emulator.html

Gavin Crate's various emulators based on microcode and Julian Highfield's
emulator, which may be found at
https://sites.google.com/site/transputeremulator/Home

Andras Pahia's continuation of this emulator which may be found at
https://github.com/pahihu/t4

Andrew Menadue's 'picoputer' extension of Andras' emulator to run on the Raspberry Pi Pico,
connected to real Inmos IMSC011 link adapters; this may be found at
https://github.com/blackjetrock/picoputer

Henry S. Warren, for "Hacker's Delight".

Pete Warden for his investigation and documentation into the Raspberry Pi Pico
memory layout at
https://petewarden.com/2024/01/16/understanding-the-raspberry-pi-picos-memory-layout/

Yury Shevchuk and Roman Pozlevich for the gcc-t800 port, ttools and libxputer
packages.  These were used throughout an earlier version of the project, for
bootstrap code, object and executable file format and loader.


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

"Hacker's Delight, second edition, Henry S. Warren"
  https://www.pearson.com/en-us/subject-catalog/p/hackers-delight/P200000000672/9780321842688

# License, Copyright & Contact info
This code is released under the Apache 2.0 License: http://www.apache.org/licenses/LICENSE-2.0.html.

(C) 2005-2024 Matt J. Gumbley

matt.gumbley@devzendo.org

Mastodon: @M0CUV@mastodon.radio

http://devzendo.github.io/parachute

