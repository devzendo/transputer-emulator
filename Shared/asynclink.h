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
#include "misc.h"
#include "log.h"

/* Lowest level abstraction: TxRxPin, represents a pair of abstract pins.
 *
 * GPIOTxRxPin would use Pi Pico GPIO pins.
 *
 * Tests would use a CrosswiredTxRxPinPair, which gives a pair of TxRxPins, A and B,
 * where setting A's Tx pin enables B's Rx pin. Setting B's Tx enables A's Rx. An AsyncLink
 * would take a TxRxPin, and tests would create two AsyncLinks with the two TxRxPins back-to-back.
 *
 * We are using oversampling to take multiple samples per bit, with samples 7, 8 and 9 being used to facilitate
 * majority voting in determining the actual state of a bit. This is achieved by the decorator implementation
 * OversampledTxRxPin which uses an underlying TxRxPin as input and whose output gives solid bit-long values
 * based on the majority vote. There are some small trade-offs in this approach documented in its implementation.
 * Note that TxRxPin has no explicit clock()/poll() method, but it's Rx/Tx methods will be called by higher level
 * abstractions during a clock poll.
 */
class TxRxPin {
public:
    virtual ~TxRxPin() = default;
    virtual bool getRx() = 0;
    virtual void setTx(bool state) = 0;
};

class OversampledTxRxPin final : TxRxPin {
public:
    explicit OversampledTxRxPin(TxRxPin& tx_rx_pin);
    ~OversampledTxRxPin() override = default;
    bool getRx() override;
    void setTx(bool state) override;

    // Used by tests - internal, do not use.
    int _resync_in_samples() const;

private:
    TxRxPin & m_pin;
    int m_resync_in_samples;
    bool m_previous_rx;
};


/* Highest level abstraction: AsyncLink, a state machine that uses the DataAckSender/Receiver (see asynclink.cpp)
 * to handle the send/receive over a TxRxPin.
 */
class AsyncLink : public Link {
public:
    AsyncLink(int linkNo, bool isServer, TxRxPin& tx_rx_pin);
    void initialise(void);
    ~AsyncLink(void);
    BYTE8 readByte(void);
    void writeByte(BYTE8 b);
    void resetLink(void);
    int getLinkType(void);
    void poll(void);
private:
    TxRxPin & m_pin;
    WORD32 myWriteSequence, myReadSequence;
};

/* Medium level abstraction: DataAckSender/Receiver. Internally used by AsyncLink.
 *
 * DataAckSender is a state machine that uses the Tx half of a TxRxPin to clock out an Ack or Data frame, can be
 * queried for its state and will notify a client (the AsyncLink) that sent Data has been Acked, or the send has timed
 * out.
 *
 * DataAckReceiver is a state machine that senses the Rx half of a TxRxPin to clock in any received Ack and/or Data
 * frame. It notifies a client (the DataAckSender) of any received Ack, and notifies a client (the DataAckSender) of any
 * received Data (so it can initiate sending an Ack).
 */
enum DataAckSenderState { IDLE, SENDING };

class DataAckSender {
public:
    explicit DataAckSender(TxRxPin& tx_rx_pin);
    DataAckSenderState state();
    void sendAck();
    void sendData(BYTE8 byte);
    void clock();

    // Used by tests - internal, do not use.
    int _queueLength();
    WORD16 _data();

private:
    TxRxPin & m_pin;
    DataAckSenderState m_state;
    int m_sampleCount;
    int m_bits;
    WORD16 m_data;
};


#endif // _ASYNCLINK_H
