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
    AsyncLink(int linkNo, bool isServer); // TODO add a TxRxPin!
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
