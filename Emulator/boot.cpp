//------------------------------------------------------------------------------
//
// File        : boot.cpp
// Description : Handles the peek/poke/boot protocol over the links to the
//               memory.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 19/04/2026
//
// (C) 2005-2026 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

// using namespace std;

#include <exception>
using namespace std;

#include "boot.h"
#include "flags.h"
#include "log.h"

Boot::Boot() = default;

Boot::~Boot() = default;

bool Boot::initialise(Memory *memory, Link *links[4]) {
    myMemory = memory;
    for (int i = 0; i < 4; i++) {
        myLinks[i] = links[i];
    }
    return true;
}

// See TTH, p53
// Transputer Instruction Set - Appendix ("start") states that the first link to receive a
// byte handles the boot/peek/poke protocol, and that the links are polled in a
// repeating cycle starting at link 0. Any number of sequential peeks/pokes are allowed
// down any link before bootstrap takes place.
// Note that Parachute is not a microcode emulator, and does not have a reset or
// analyse 'pin'.
void Boot::start() {
    // TODO make use of multiple links, later.
	int linkNo = 0;
	Link *bootLink = myLinks[linkNo];

    BYTE8 bootLen = 0;
    BYTE8 ctrl = 0;
    // Repeatedly read first byte:
    // 'poke': 0 => read address word, data word, store data word at address
    // 'peek': 1 => read word, output word at that address
    // 'boot': x where x>1, x is the length of boot code to read into MemStart onwards
    // CWG p74 states that the address does not need to be word-aligned.
    do {
        try {
            ctrl = bootLink->readByte();
            if (IS_FLAG_SET(DebugFlags_LinkComms)) {
                logDebugF("Boot ctrl byte = %02X", ctrl);
            }
            switch (ctrl) {
                case BOOT_PEEK: // peek
                    try {
                        WORD32 addr = bootLink->readWord();
                        WORD32 value = 0xDEADF00D;
                        if (myMemory->isLegalMemory(addr)) {
                            value = myMemory->getWord(addr);
                            if (IS_FLAG_SET(DebugFlags_LinkComms)) {
                                logDebugF("Boot-peek @ %08X = %08X", addr, value);
                            }
                        } else {
                            logWarnF("Boot-peek requested read from bad address %08X", addr);
                        }
                        bootLink->writeWord(value);
                    } catch (exception &e) {
                        logFatalF("I/O failure on link %d during boot-peek: %s", linkNo, e.what());
                        exit(1);
                    }
                    break;
                case BOOT_POKE:
                    try {
                        WORD32 addr = bootLink->readWord();
                        WORD32 value = bootLink->readWord();
                        if (myMemory->isLegalMemory(addr)) {
                            myMemory->setWord(addr, value);
                            if (IS_FLAG_SET(DebugFlags_LinkComms)) {
                                logDebugF("Boot-poke stored %08X @ %08X", value,  addr);
                            }
                        } else {
                            logWarnF("Boot-poke requested write to bad address %08X value %08X", addr, value);
                        }
                    } catch (exception &e) {
                        logFatalF("I/O failure on link %d during boot-poke: %s", linkNo, e.what());
                        exit(1);
                    }
                    break;

            }
        } catch (exception &e) {
            logFatalF("Boot failed to read control byte from link %d: %s", linkNo, e.what());
            exit(1);
        }
    } while (ctrl <= 1);
    logDebug("Boot done");
}
