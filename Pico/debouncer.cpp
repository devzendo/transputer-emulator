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

#include <cstdint>

#include "debouncer.h"

constexpr uint8_t checkMsec = 5;     // Read hardware every so many milliseconds
constexpr uint8_t pressMsec = 10;    // Stable time before registering pressed
constexpr uint8_t releaseMsec = 20;  // Stable time before registering released

Debouncer::Debouncer() {
    debouncedKeyPress = false; // If using internal pullups, the initial state is true.
    count = releaseMsec / checkMsec;
    keyChanged = false;
    keyReleased = false;
}

// called every checkMsec.
// The key state is +5v=released, 0v=pressed; there are pullup resistors.
void Debouncer::debounce(const bool rawPinState) {
    keyChanged = false;
    keyReleased = debouncedKeyPress;
    if (rawPinState == debouncedKeyPress) {
        // Set the timer which allows a change from current state
        resetTimer();
    } else {
        if (--count == 0) {
            // key has changed - wait for new state to become stable
            debouncedKeyPress = rawPinState;
            keyChanged = true;
            keyReleased = debouncedKeyPress;
            // And reset the timer
            resetTimer();
        }
    }
}

void Debouncer::resetTimer() {
    if (debouncedKeyPress) {
        count = releaseMsec / checkMsec;
    } else {
        count = pressMsec / checkMsec;
    }
}


