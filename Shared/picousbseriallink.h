//------------------------------------------------------------------------------
//
// File        : picousbseriallink.h
// Description : A Link over the Pi Pico's USB Serial connection.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 07/10/2023
//
// (C) 2005-2025 Matt J. Gumbley
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
    void initialise(void);
    ~PicoUSBSerialLink(void);
    BYTE8 readByte(void);
    void writeByte(BYTE8 b);
    void resetLink(void);
    int getLinkType(void);
	void poll(void);
private:
    WORD32 myWriteSequence, myReadSequence;
};

#endif // _PICOUSBSERIALLINK_H
