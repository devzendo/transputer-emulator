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
 * Note that TxRxPin has no explicit clock()/poll() method, but its Rx/Tx methods will be called by higher level
 * abstractions during a clock poll.
 */
class TxRxPin {
public:
    virtual ~TxRxPin() = default;
    virtual bool getRx() = 0;
    virtual void setTx(bool state) = 0;
};

/*
 * The OversampledTxRxPin will emit majority-voted bit states via an instance of this interface that can be registered.
 */
class RxBitReceiver {
public:
    virtual ~RxBitReceiver() = default;
    virtual void bitStateReceived(bool state) = 0;
};

class OversampledTxRxPin final : TxRxPin {
public:
    explicit OversampledTxRxPin(TxRxPin& tx_rx_pin);
    ~OversampledTxRxPin() override = default;
    bool getRx() override;
    void setTx(bool state) override;

    void registerRxBitReceiver(RxBitReceiver& rxBitReceiver) const;
    void unregisterRxBitReceiver() const;

    // Used by tests - internal, do not use.
    int _resync_in_samples() const;

private:
    TxRxPin & m_pin;
    int m_resync_in_samples, m_sample_index, m_data_bits_length;
    WORD16 m_data_samples;
    WORD16 m_data_bits;

    bool m_previous_rx;
    bool m_latched_output_rx;

    mutable RxBitReceiver* m_rx_bit_receiver;
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

/*
 * Callbacks used between DataAckSender/Receiver
 */

// TODO CONSIDER: do we need two interfaces for the DAR's notifications? Could they be merged?

// The DataAckReceiver will call a registered instance of AckReceiver when it detects
// an incoming ack.
class AckReceiver {
public:
    virtual ~AckReceiver() = default;
    virtual void ackReceived() = 0;
};

// The DataAckReceiver will call a registered instance of SendAckReceiver's requestSendAck() when it
// starts to receive data bits. The implementation of SendAckReceiver will return true if there is sufficient space
// in its receive buffer (and in the case of the DataAckSender, will start to send an ack), or false if
// there is not (if no client has read a previously-received byte: an overrun) (in which case,
// this will be notified to the DataAckSender's client, the AsyncLink).
class SendAckReceiver {
public:
    virtual ~SendAckReceiver() = default;
    /**
     * Data is being received, is there enough room in the receive buffer to hold it? Return true if so, and send an
     * ack. If not enough space, return false and do not send an ack.
     */
    virtual bool requestSendAck() = 0;
};

// The DataAckReceiver will call a registered instance of FramingErrorReceiver's framingError() when it receives
// a high input when in STOP_BIT state.
class FramingErrorReceiver {
public:
    virtual ~FramingErrorReceiver() = default;
    /**
     * A framing error (bad stop bit) has occurred.
     */
    virtual void framingError() = 0;
};

// The DataAckReceiver will call a registered instance of DataReceiver's dataReceived() when it receives a byte of data
// followed by a valid stop bit.
class DataReceiver {
public:
    virtual ~DataReceiver() = default;
    /**
     * A byte of data has been received.
     */
    virtual void dataReceived(BYTE8 data) = 0;
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
enum class DataAckSenderState { IDLE, SENDING };

class DataAckSender: public AckReceiver {
public:
    explicit DataAckSender(TxRxPin& tx_rx_pin);
    DataAckSenderState state() const;
    void sendAck();
    void sendData(BYTE8 byte);
    void clock();

    void changeState(DataAckSenderState newState);
    void ackReceived() override;

    // Used by tests - internal, do not use.
    int _queueLength() const;
    WORD16 _data() const;
    bool _send_ack() const;
    bool _ack_rxed() const;

private:
    TxRxPin & m_pin;
    DataAckSenderState m_state;
    bool m_send_ack;
    bool m_ack_rxed;
    int m_sampleCount;
    int m_bits;
    WORD16 m_data;
};

enum class DataAckReceiverState { IDLE, START_BIT_2, DATA, DISCARD, STOP_BIT };

class DataAckReceiver : public RxBitReceiver {
public:
    explicit DataAckReceiver(TxRxPin& tx_rx_pin);
    DataAckReceiverState state() const;
    void bitStateReceived(bool state) override;

    void registerAckReceiver(AckReceiver& ackReceiver) const;
    void unregisterAckReceiver() const;
    void registerSendAckReceiver(SendAckReceiver& sendAckReceiver) const;
    void unregisterSendAckReceiver() const;
    void registerFramingErrorReceiver(FramingErrorReceiver& framingErrorReceiver) const;
    void unregisterFramingErrorReceiver() const;
    void registerDataReceiver(DataReceiver& dataReceiver) const;
    void unregisterDataReceiver() const;

    void changeState(DataAckReceiverState newState);

    // Internal, used by tests, do not use
    int _bit_count() const;
    BYTE8 _buffer() const;

private:
    TxRxPin & m_pin;
    DataAckReceiverState m_state;
    int m_bit_count;
    BYTE8 m_buffer;

    mutable AckReceiver *m_ack_receiver = nullptr;
    mutable SendAckReceiver *m_send_ack_receiver = nullptr;
    mutable FramingErrorReceiver *m_framing_error_receiver = nullptr;
    mutable DataReceiver *m_data_receiver = nullptr;
};

#endif // _ASYNCLINK_H
