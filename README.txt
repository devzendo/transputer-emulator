README.txt
----------

This is the source code distribution of the Parachute parallel programming
environment.

(C) 2005-2018 Matt J. Gumbley
matt.gumbley@gmail.com
@mattgumbley @devzendo
http://devzendo.github.io/dev/parachute

LICENSING
---------
This software is distributed under the GNU General Public License - see COPYING
for more details.

DIRECTORY STRUCTURE
-------------------
The source is split into the following directories:

ToolChain - the GCC-T800 GCC 2.7.2, TTOOLS 2.0beta2, libxputer and dasm
packages, plus packages which we use to build the C Startup code and client
libraries. TTOOLS is also used by the languages developed as part of this
project.

Shared - utility code that is common to many parts of the system.

NodeServer - the client and server portions of the node server and its protocol
definition. The server runs on your host computer (i.e. under Windows, Linux,
Mac OS X etc.) and the client runs under T800 emulation as part of your
application.

Emulator - the T800 emulator.

Finally there is the documentation:
Notes - various notebooks I kept during development.

Reference - datasheets for the T805, Prof. Hoare's book on CSP, various
documents on Transputer programming, languages, compilers, etc.

BUILDING AND INSTALLING
-----------------------
The distribution currently builds under the following systems:
* Mac OSX 'El Capitan' 10.11
* Ubuntu Linux 16.04.4 LTS Intel x86-64
* CentOS Linux 7.4 Intel x86-64
* Windows 10

Prerequisites:
- Ubuntu Linux: flex, build-essential


The install location is /usr/local/t800. Changing this would entail changes to
Makefiles in the above hierarchy, and changes to *.parachute.patch in
ToolChain.

To build, cd to the top level directory (where this README.txt is) and do:
make

During the build, you will be prompted for a password. This is since the files
are installed as root, and the build requires a sudo password.

This build will build the entire system, toolchain, T800 emulator and node
server, client libraries, etc.


MAILING LIST
------------
Groups.io

... to be continued ...




