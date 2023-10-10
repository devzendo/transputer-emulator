//------------------------------------------------------------------------------
//
// File        : picousbseriallink.h
// Description : A Link over the Pi Pico's USB Serial connection.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 07/10/2023
//
// (C) 2005-2023 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _PICOUSBSERIALLINK_H
#define _PICOUSBSERIALLINK_H

#include <exception>

#include "types.h"
#include "link.h"

class PicoUSBSerialLink : public Link {
public:
    PicoUSBSerialLink(int linkNo, bool isServer);
    void initialise(void) throw (std::exception);
    ~PicoUSBSerialLink(void);
    BYTE8 readByte(void) throw (std::exception);
    void writeByte(BYTE8 b) throw (std::exception);
    void resetLink(void) throw (std::exception);
    int getLinkType(void);
private:
    WORD32 myWriteSequence, myReadSequence;
};

#endif // _PICOUSBSERIALLINK_H
