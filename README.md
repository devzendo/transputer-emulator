transputer-emulator
===================
This is an emulator of the Inmos T800 Transputer, and Node Server that interfaces it
to a host OS, providing boot/debug/IO facilities. It is part of the
[Parachute Project](https://devzendo.github.io/parachute). 

(C) 2005-2018 Matt J. Gumbley
matt.gumbley@devzendo.org
@mattgumbley @devzendo
http://devzendo.github.io/parachute

Status
------
In progress, June 2018. Modernising, building on modern OSX/Linux/Windows.

Remaining work:
* Fix all compiler warnings (some tautologous comparisons remain)
* Determine calling convention for node server client library & understanding of C->.S conversion, rewrite as .ASM in
  tmasm format.
* Node Server client library in tmasm assembler
* Develop debug interface
* Write debugger program
* Node server needs to support terminal I/O facilities (no echo key reads, sensing available readable keys) for eForth
* Unfinished: resetch when given a link not a memory channel
* Refactoring: make use of isLegalMemory within memory.cpp

* Convert to CMake for use in CLion
* Build on Linuxes

Future intentions:
* Replace NodeServer with an iserver-protocol compatible version
* Remove potential buffer overflows in cpu.cpp
* Add remaining T800/T805 instructions
* Emulate multiple Transputers on 1-N physical cores
* Link topology management
* Investigate Benes networks
* Add memory-mapped frame buffer via SDL
* Add mouse interface for same
* Build on Windows

Bugs:
* (possibly obviated by CMake) why does 'make test' in Shared says it fail to link, yet actually does, and works?
 
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
* Ubuntu Linux 16.04.4 LTS Intel x86-64 (future work)
* CentOS Linux 7.4 Intel x86-64 (future work)
* Windows 10 (future work)

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



