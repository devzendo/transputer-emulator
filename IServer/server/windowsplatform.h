//------------------------------------------------------------------------------
//
// File        : windowsplatform.h
// Description : Definition of Windows Console/Timer.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 05/03/2019
//
// (C) 2005-2020 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _WINDOWSPLATFORM_H
#define _WINDOWSPLATFORM_H

#include <exception>

#include "types.h"
#include "platform.h"

class WindowsPlatform final : public Platform {
public:
    WindowsPlatform();
    void initialise() noexcept(false) override;
    ~WindowsPlatform() override;

    bool isConsoleCharAvailable() override;
    BYTE8 getConsoleChar() override;
    void putConsoleChar(BYTE8 ch) override;

    WORD32 getTimeMillis() override;
    UTCTime getUTCTime() override;
private:
};

#endif // _WINDOWSPLATFORM_H

