//------------------------------------------------------------------------------
//
// File        : asynclink.h
// Description : An asynchronous link that works with a pair of (abstract)
//               GPIO pins.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 26/09/2025
//
// (C) 2005-2025 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _ASYNCLINK_H
#define _ASYNCLINK_H

#include "types.h"
#include "link.h"
#include "log.h"

/*
 * Using oversampling: The clock pulse triggers 16 times the bit frequency, giving 16 samples per bit.
 * |         1111111|
 * |1234567890123456|
 *        xxx
 * Only samples 7, 8 and 9 are read, and a majority vote taken to determine the actual value of the
 * transmitted bit.
 */

/* Lowest level abstraction: TxRxPin, represents a pair of abstract pins. GPIOTxRxPin would use Pi Pico
 * GPIO pins. Tests would use a CrosswiredTxRxPinPair, which gives a pair of TxRxPins, A and B,
 * where setting A's Tx pin enables B's Rx pin. Setting B's Tx enables A's Rx. An AsyncLink
 * would take a TxRxPin, and tests would create two AsyncLinks with the two TxRxPins back-to-back.
 */
class TxRxPin {
public:
    virtual ~TxRxPin() {}
    virtual bool getRx() = 0;
    virtual void setTx(bool state) = 0;
};

/* Highest level abstraction: AsyncLink, a state machine that uses the DataAckSender/Receiver
 */
class AsyncLink : public Link {
public:
    AsyncLink(int linkNo, bool isServer);
    void initialise(void);
    ~AsyncLink(void);
    BYTE8 readByte(void);
    void writeByte(BYTE8 b);
    void resetLink(void);
    int getLinkType(void);
    void poll(void);
private:
    WORD32 myWriteSequence, myReadSequence;
};

/* Medium level abstraction: DataAckSender/Receiver, a state machine that uses the Tx half of
 * a TxRxPin to clock out an Ack or Data frame and can be queried for its state. DataAckReceiver,
 * a state machine that senses the Rx half of a TxRxPin to clock in any received Ack and/or Data frame.
 */

// TODO move this implementation to the .cpp

enum DataAckSenderState { IDLE, SENDING };

class DataAckSender {
public:
    DataAckSender(TxRxPin& tx_rx_pin) : m_pin(tx_rx_pin) {
        // TODO mutex {
        m_state = IDLE;
        // TODO }
    }

    DataAckSenderState state() {
        // TODO mutex {
        return m_state;
        // TODO }
    }

    void sendAck() {
        // TODO if m_state != IDLE throw up
        // TODO mutex {
        m_sampleCount = 0;
        m_bits = 2;
        m_data = 0x01;
        m_state = SENDING;
        // TODO }
    }

    void clock() {
        //logDebugF("clock > %d sample count %d bits %d data 0x%04X", m_state, m_sampleCount, m_bits, m_data);
        // TODO mutex {
        switch (m_state) {
            case IDLE:
                break;
            case SENDING:
                const bool one = m_data & 0x0001;
                m_pin.setTx(one);
                m_sampleCount ++;
                if (m_sampleCount == 16) {
                    m_sampleCount = 0;
                    m_bits--;
                    m_data >>= 1;
                    if (m_bits == 0) {
                        m_state = IDLE;
                    }
                }
                break;
        }
        // TODO }
        //logDebugF("clock < %d sample count %d bits %d data 0x%04X", m_state, m_sampleCount, m_bits, m_data);
    }

    int _queueLength() {
        return m_bits;
    }

private:
    TxRxPin & m_pin;
    DataAckSenderState m_state;
    int m_sampleCount;
    int m_bits;
    WORD16 m_data;
};


#endif // _ASYNCLINK_H
