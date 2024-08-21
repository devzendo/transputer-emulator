//------------------------------------------------------------------------------
//
// File        : tvslink.cpp
// Description : A Link that's only used by the emulator to send program and
//               input data to itself, and for the running program to send
//               output data to the output file.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 13/09/2023
//
// (C) 2005-2023 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <exception>
#include <cctype>

#include "tvslink.h"
#include "log.h"

TVSLink::TVSLink(int linkNo, std::string tvsProgram, std::string tvsInput, std::string tvsOutput) :
    Link(linkNo, false) {
    myTVSProgram = tvsProgram;
    myTVSInput = tvsInput;
    myTVSOutput = tvsOutput;
    myProgramSent = myInputSent = 0;
    logDebugF("Constructing TVS link %d for cpu client", myLinkNo);
}

void TVSLink::initialise(void) {
    static char msgbuf[255];
    logDebugF("Initialising TVS link %d for cpu client", myLinkNo);
    myWriteSequence = myReadSequence = 0;
    try {
        myTVSProgramStream.open(myTVSProgram, std::ifstream::in | std::ifstream::binary);
    } catch (std::system_error& e) {
        sprintf(msgbuf, "Could not open program file %s: %s", myTVSProgram.c_str(), e.code().message().c_str());
        logFatalF("%s", msgbuf);
        throw std::runtime_error(msgbuf);
    }
    if (!myTVSInput.empty()) {
        try {
            myTVSInputStream.open(myTVSInput, std::ifstream::in | std::ifstream::binary);
        } catch (std::system_error& e) {
            sprintf(msgbuf, "Could not open input file %s: %s", myTVSInput.c_str(), e.code().message().c_str());
            logFatalF("%s", msgbuf);
            throw std::runtime_error(msgbuf);
        }
    } else {
        logDebug("There is no TVS input file");
    }
    try {
        myTVSOutputStream.open(myTVSOutput, std::ofstream::out | std::ofstream::binary);
    } catch (std::system_error& e) {
        sprintf(msgbuf, "Could not open output file %s: %s", myTVSOutput.c_str(), e.code().message().c_str());
        logFatalF("%s", msgbuf);
        throw std::runtime_error(msgbuf);
    }
}

TVSLink::~TVSLink() {
    logDebugF("Destroying TVS link %d", myLinkNo);
    myTVSProgramStream.close();
    if (!myTVSInput.empty()) {
        myTVSInputStream.close();
    }
    myTVSOutputStream.close();
}

BYTE8 TVSLink::readByte() {
    BYTE8 buf[1];
    bool finished = false;
    myTVSProgramStream.read(reinterpret_cast<char *>(buf), 1);
    if (myTVSProgramStream.gcount() == 0) {
        myTVSProgramStream.close();
        if (myTVSInput.empty()) {
            logInfo("Program is at EOF; there is no input");
            finished = true;
        } else {
            myTVSInputStream.read(reinterpret_cast<char *>(buf), 1);

            if (myTVSInputStream.gcount() == 0) {
                myTVSInputStream.close();
                logInfo("Program and input files are both at EOF");
                finished = true;
            } else {
                myInputSent++;
                if (bDebug) {
                    logDebugF("Read input byte %08x...", myInputSent);
                }
            }
        }
    } else {
        myProgramSent++;
        if (bDebug) {
            logDebugF("Read program byte %08x...", myProgramSent);
        }
    }
    if (finished) {
        logInfo("Finished; terminating emulator");
        throw std::runtime_error("TVS signalled end of emulation");
    }

    if (bDebug) {
        logDebugF("Link %d R #%08X %02X (%c)", myLinkNo, myReadSequence++, buf[0], isprint(buf[0]) ? buf[0] : '.');
    }
    return buf[0];
}

void TVSLink::writeByte(BYTE8 buf) {
    if (bDebug) {
        logDebugF("Link %d W #%08X %02X (%c)", myLinkNo, myWriteSequence++, buf, isprint(buf) ? buf : '.');
    }
    const char outBuf = buf;
    myTVSOutputStream.write(reinterpret_cast<const char *>(&outBuf), 1);
    myTVSOutputStream.flush();
}

void TVSLink::resetLink(void) {
    // TODO
}

int TVSLink::getLinkType() {
    return LinkType_TVS;
}
