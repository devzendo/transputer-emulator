//------------------------------------------------------------------------------
//
// File        : debouncer.cpp
// Description : Software digital input debouncing, based on code by Jack
//               Ganssle, http://www.ganssle.com/debouncing-pt2.htm
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 28/11/2025
//
// (C) 2005-2025 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _DEBOUNCER_H
#define _DEBOUNCER_H

class Debouncer {
public:
    Debouncer();

    // called every checkMsec.
    // The key state is +5v=released, 0v=pressed; there are pullup resistors.
    void debounce(bool rawPinState);

    // Signals the key has changed from open to closed, or the reverse.
    bool keyChanged;
    // The current debounced state of the key.
    bool keyReleased;

private:
    void resetTimer();
    uint8_t count;
    // This holds the debounced state of the key.
    bool debouncedKeyPress;
};

#endif // _DEBOUNCER_H