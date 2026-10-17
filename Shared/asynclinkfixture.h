//------------------------------------------------------------------------------
//
// File        : asynclinkfixture.h
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

#ifndef ASYNCLINKFIXTURE_H
#define ASYNCLINKFIXTURE_H

#include <atomic>

#include "gpioasynclink.h"




class CrosswiredPin: public TxRxPin {
public:
    CrosswiredPin(const char *side, std::atomic_bool *rxstate, std::atomic_bool *txstate);
    ~CrosswiredPin() override = default;

    bool getRx() override;
    void setTx(bool state) override;

private:
    std::atomic_bool *m_rxstate;
    std::atomic_bool *m_txstate;
    char m_side = ' ';
};

class CrosswiredTxRxPinPair {
public:
    CrosswiredTxRxPinPair() :
        m_aPinEnd(CrosswiredPin("A", &aPin, &bPin)),
        m_bPinEnd(CrosswiredPin("B", &bPin, &aPin)) {
    }
    ~CrosswiredTxRxPinPair() = default;

    TxRxPin & pairA();
    TxRxPin & pairB();

private:
    CrosswiredPin m_aPinEnd;
    CrosswiredPin m_bPinEnd;
    std::atomic_bool aPin{false};
    std::atomic_bool bPin{false};
};

#endif // ASYNCLINKFIXTURE_H
