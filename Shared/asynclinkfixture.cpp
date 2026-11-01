//------------------------------------------------------------------------------
//
// File        : asynclinkfixture.cpp
// Description : Test fixture classes shared between multiple tests for async
//               link testing.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 17/10/2026
//
// (C) 2005-2026 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include "asynclinkfixture.h"
#include "log.h"
CrosswiredPin::CrosswiredPin(const char *side, std::atomic_bool *rxstate, std::atomic_bool *txstate) {
    m_side = *side;
    m_rxstate = rxstate;
    m_txstate = txstate;
}

bool CrosswiredPin::getRx() {
    // logDebug("CrosswiredPin: getRx load...");
    const bool retval = m_rxstate->load();
    // logDebugF("RX %c pin state is %d", m_side, retval);
    return retval;
}

void CrosswiredPin::setTx(const bool state) {
    //logDebugF("Setting TX %c pin state to %d", m_side, state);
    m_txstate->store(state);
}


TxRxPin & CrosswiredTxRxPinPair::pairA() {
    return reinterpret_cast<TxRxPin&>(m_aPinEnd);
};

TxRxPin & CrosswiredTxRxPinPair::pairB() {
    return reinterpret_cast<TxRxPin&>(m_bPinEnd);
};


/**
 * A MultipleAsyncLinkClocker clocks all the Links it is given.
 */
MultipleClockableClocker::MultipleClockableClocker() : TickHandler() {

}

void MultipleClockableClocker::addClockable(Clockable* clockable) {
    m_clockables.push_back(clockable);
}

// static bool tickled = false;
// static int tick_count = 0;
// TickHandler
void MultipleClockableClocker::tick() {
    // if (tick_count++ == 1024) {
    //     tick_count = 0;
    //     gpio_put(PICO_DEFAULT_LED_PIN, tickled);
    //     tickled = ! tickled;
    // }

    for (Clockable* clockable: m_clockables) {
        clockable->clock();
    }
}
