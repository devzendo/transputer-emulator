transputer-emulator
===================
This repository contains the source code distribution of the Parachute parallel programming
environment - an emulator of the Inmos T800 Transputer, and Node Server that interfaces it
to a host OS, providing boot/debug/IO facilities.

(C) 2005-2018 Matt J. Gumbley
matt.gumbley@devzendo.org
@mattgumbley @devzendo
http://devzendo.github.io/parachute

Status
------
In progress, June 2018. Modernising, building on modern OSX/Linux/Windows.


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
- OSX: clang, GNU make (e.g. via XCode Developer tools, or MacPorts)
- Ubuntu Linux: build-essential (=> gcc, GNU Make)
- CentOS: gcc, make
- Windows: (future work)


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



