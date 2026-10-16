//------------------------------------------------------------------------------
//
// File        : systemlink.h
// Description : Definition of Operating System Links (as opposed to Interrupt-
//               Driven Links: see asynclink.h).
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 16/10/2026
//
// (C) 2005-2026 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef SYSTEMLINK_H
#define SYSTEMLINK_H

#include "link.h"

class SystemLink: public Link {
public:
    SystemLink(int linkNo, bool isServer);
    ~SystemLink() override;

    bool writeDataAsync(WORD32 workspacePointer, BYTE8* dataPointer, WORD32 length) override;
    void readDataAsync(WORD32 workspacePointer, BYTE8* dataPointer, WORD32 length) override;
	WORD32 writeComplete() override;
    WORD32 readComplete() override;

};

#endif // SYSTEMLINK_H
