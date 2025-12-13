//------------------------------------------------------------------------------
//
// File        : gpioasynclink.h
// Description : An asynchronous link that works with a pair of (abstract)
//               GPIO pins. This will be low-speed, nowhere near the speed
//               of real Transputer links - a PIO version will hopefully
//               achieve this.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 26/09/2025
//
// (C) 2005-2025 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _GPIOASYNCLINK_H
#define _GPIOASYNCLINK_H

#include <atomic>
#include <mutex> // For std::lock_guard and BasicLockable
#include <vector>

#ifdef DESKTOP
#include <thread>
#endif

#ifdef PICO
#include <pico/sync.h>
#endif

#include <sys/types.h>
#include "types.h"
#include "link.h"
#include "sync.h"
#include "asynclink.h"

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
 * Note that TxRxPin has no explicit clock() method, but its Rx/Tx methods will be called by higher level
 * abstractions during a clock tick.
 */

class TxRxPin {
public:
    virtual ~TxRxPin() = default;
    virtual bool getRx() = 0;
    virtual void setTx(bool state) = 0;
};

/*
 * A TxRxPin that uses a pair of Pico GPIO pins for transmit and receive.
 */
#ifdef PICO
class GPIOTxRxPin: public TxRxPin {
public:
    GPIOTxRxPin(uint txGPIOPin, uint rxGPIOPin);
    ~GPIOTxRxPin() override = default;
    bool getRx() override;
    void setTx(bool state) override;
private:
    uint m_txGPIOPin, m_rxGPIOPin;
};
#endif // PICO

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

/*
 * Callbacks/facades used between DataAckSender->AsyncLink and DataAckReceiver->DataAckSender and
 * DataAckReceiver->AsyncLink.
 */

/**
 * The DataAckSender will call a registered instance of SenderToLink to query/set/clear the AsyncLink's Ready To Send
 * flag, and to set the timeout error flag.
 */

class SenderToLink {
public:
    virtual ~SenderToLink() = default;

    /**
     * The DataAckSender needs to know the state of the AsyncLink's Ready To Send flag.
     */
    virtual bool queryReadyToSend() = 0;

    /**
     * The DataAckSender needs to set and clear the AsyncLink's Ready To Send flag.
     */
    virtual void setReadyToSend() = 0;
    virtual void clearReadyToSend() = 0;

    /**
     * The DataAckSender has sent some data, but has not received an ack, so the send operation has timed out: set
     * this flag in the AsyncLink.
     */
    virtual void setTimeoutError() = 0;
};

/**
 * The DataAckReceiver will call a registered instance of ReceiverToSender's sendAck() when it
 * starts to receive data bits, and if the Link's Read Data Available flag is true (i.e. there's space in the reception
 * buffer).
 * The ackReceived() method will be called when an ack has been detected. This will eventually cause the Link's Ready To
 * Send flag to be cleared, indicating that more data can be sent.
 */

class ReceiverToSender {
public:
    virtual ~ReceiverToSender() = default;

    /**
     * Data is being received, and there is room in the reception buffer to hold it, so send an
     * ack.
     */
    virtual void sendAck() = 0;

    /**
     * An ack has been received.
     */
    virtual void ackReceived() = 0;
};

/**
 * The DataAckReceiver has this facade on to the AsyncLink to inform it of events.
 */

class ReceiverToLink {
public:
    virtual ~ReceiverToLink() = default;
    /**
     * The DataAckReceiver has detected a framing error (high input when in STOP_BIT state), and informs the AsyncLink
     * via this method.
     */
    virtual void framingError() = 0;

    /**
     * The DataAckReceiver has detected an overrun error (data was being received but the reception buffer is full: the
     * Read Data Available flag is true), so informs the AsyncLink via this method.
     */
    virtual void overrunError() = 0;

    /**
     * The DataAckReceiver has received a byte of data (followed by a valid stop bit) to give to the AsyncLink. This
     * will cause the AsyncLink to set its Read Data Available flag.
     */
    virtual void dataReceived(BYTE8 data) = 0;

    /**
     * The DataAckReceiver needs to know the state of the AsyncLink's Read Data Available flag.
     */
    virtual bool queryReadDataAvailable() = 0;

    /**
     * The DataAckReceiver wants to clear the AsyncLink's Read Data Available flag - it does this on initialisation.
     */
    virtual void clearReadDataAvailable() = 0;
};

/* Medium level abstraction: DataAckSender/Receiver. Internally used by AsyncLink.
 *
 * DataAckSender is a state machine that uses the Tx half of a TxRxPin to clock out an Ack or Data frame, can be
 * queried for its state and will notify a client (the AsyncLink) that sent Data has been Acked, or the send operation
 * has timed out.
 *
 * DataAckReceiver is a state machine that senses the Rx half of a TxRxPin to clock in any received Ack and/or Data
 * frame. It notifies a client (the DataAckSender) of any received Ack, and notifies a client (the DataAckSender) of any
 * received Data (so it can initiate sending an Ack).
 */

enum class DataAckSenderState { IDLE, SENDING_ACK, SENDING_DATA, ACK_TIMEOUT };
const char* DataAckSenderStateToString(DataAckSenderState s) noexcept;

class DataAckSender: public ReceiverToSender {
public:
    explicit DataAckSender(int linkNo, TxRxPin& tx_rx_pin);

    /**
     * Send data
     * @param byte data to send
     * @return true iff we are in a state where data can be sent.
     */
    bool sendData(BYTE8 byte);

    /**
     * Clock pulse - clock data or ack out.
     */
    void clock();

    void registerSenderToLink(SenderToLink& senderToLink) const;
    void unregisterSenderToLink() const;

    void changeState(DataAckSenderState newState);

    // ReceiverToSender: the Receiver will inform us of these.
    void sendAck() override;
    void ackReceived() override;

    // Used by tests - internal, do not use.
    DataAckSenderState state() const;
    int _queueLength() const;
    WORD16 _data() const;
    bool _send_ack() const;
    bool _ack_rxed() const;
    bool _data_enqueued() const;
    BYTE8 _data_enqueued_buffer() const;

private:
    int m_linkNo;
    TxRxPin & m_pin;
    DataAckSenderState m_state;
    bool m_send_ack;
    bool m_ack_rxed;
    int m_sampleCount;
    int m_bits;
    WORD16 m_data;
    bool m_data_enqueued;
    BYTE8 m_data_enqueued_buffer;
    void sendDataInternal(BYTE8 data);
    mutable SenderToLink *m_sender_to_link = nullptr;
};

enum class DataAckReceiverState { IDLE, START_BIT_2, DATA, DISCARD, STOP_BIT };

class DataAckReceiver : public RxBitReceiver {
public:
    explicit DataAckReceiver(int linkNo);
    DataAckReceiverState state() const;
    void bitStateReceived(bool state) override;

    void registerReceiverToLink(ReceiverToLink& receiverToLink) const;
    void unregisterReceiverToLink() const;
    void registerReceiverToSender(ReceiverToSender& receiverToSender) const;
    void unregisterReceiverToSender() const;

    void changeState(DataAckReceiverState newState);

    // Internal, used by tests, do not use
    int _bit_count() const;
    BYTE8 _buffer() const;

private:
    int m_linkNo;
    DataAckReceiverState m_state;
    int m_bit_count;
    BYTE8 m_buffer;

    mutable ReceiverToSender *m_receiver_to_sender = nullptr;
    mutable ReceiverToLink *m_receiver_to_link = nullptr;
};

/* Access to the AsyncLink status word and other apparatus is protected by a std::lock_guard
 * and an appropriate lock for the platform - std::mutex on desktop, and a critical_section_t
 * on PICO, wrapped in this BasicResolvable.
 */

#ifdef DESKTOP
#define MUTEX     std::lock_guard<std::mutex> guard(m_mutex);
#endif

#ifdef PICO
#define MUTEX     std::lock_guard<CriticalSection> guard(m_criticalsection);
#endif

/*
 * A AsyncLinkClock will tick at a set frequency, and call its registered TickHandler, to 'do whatever'.
 * This is used to clock all the links.
 *
 * On the PICO, a diagnostic GPIO pin will be raised around the call to the tick handler, to
 * allow measurement of the duration of the tick handler. The SDK hardware_timer's repeating
 * timer is used.
 *
 * On DESKTOP, a std::thread is used to call the tick handler repeatedly, until terminated.
 * There's no link hardware on desktop systems, but this is used in unit tests.
 */

class TickHandler {
public:
    TickHandler() : m_is_running(false) {
    };
    virtual ~TickHandler() = default;
    virtual void tick() = 0;
    bool is_running() {
        return m_is_running.load();
    };
    void start() {
        m_is_running.store(true);
    }
    void stop() {
        m_is_running.store(false);
    }
private:
    std::atomic_bool m_is_running;
};

const int64_t LINK_CLOCK_TICK_INTERVAL_US = 50; // 0.05ms

class AsyncLinkClock {
public:
    AsyncLinkClock(uint clockGPIOPin, TickHandler& tickHandler);
    void start();
    bool is_running();
    void stop();
    ~AsyncLinkClock();
    void operator()() const; // the caller of the tick handler
private:
    void tick();
    uint m_clockGPIOPin{};
    TickHandler &m_tick_handler;
#ifdef PICO
    struct repeating_timer m_timer;
#endif
#ifdef DESKTOP
    std::thread *m_thread = nullptr;
#endif
};

struct LinkRegisters {
public:
    WORD32 m_workspace_pointer;
    BYTE8* m_data_pointer;
    WORD32 m_length;
};


/* Highest level abstraction: GPIOAsyncLink uses the DataAckSender & DataAckReceiver state
 * machines, and an OversampledTxRxPin to handle the send/receive over an underlying TxRxPin.
 */

class GPIOAsyncLink : public Link, public AsyncLink, SenderToLink, ReceiverToLink {
public:
    GPIOAsyncLink(int linkNo, bool isServer, TxRxPin& tx_rx_pin);
    void initialise() override;
    ~GPIOAsyncLink() override;
    BYTE8 readByte() override;
    void writeByte(BYTE8 b) override;
    void resetLink() override;
    int getLinkType() override;

    // AsyncLink
    void clock() override;
    bool writeDataAsync(WORD32 workspacePointer, BYTE8* dataPointer, WORD32 length) override;
    WORD32 writeComplete() override;
    void readDataAsync(WORD32 workspacePointer, BYTE8* dataPointer, WORD32 length) override;
    WORD32 readComplete() override;
    WORD16 getStatusWord() override;

    // All internals...
    // SenderToLink
    bool queryReadyToSend() override;
    void setReadyToSend() override;
    void clearReadyToSend() override;
    void setTimeoutError() override;

    // ReceiverToLink
    void framingError() override;
    void overrunError() override;
    void dataReceived(BYTE8 data) override;
    bool queryReadDataAvailable() override;
    void clearReadDataAvailable() override;

private:
    TxRxPin & m_pin;
    OversampledTxRxPin * m_o_pin;
    DataAckSender * m_sender;
    DataAckReceiver * m_receiver;
    LinkRegisters m_send_registers;
    LinkRegisters m_receive_registers;
    volatile WORD16 m_status_word;
    WORD32 myWriteSequence, myReadSequence;
#ifdef DESKTOP
    std::mutex m_mutex;
#endif
#ifdef PICO
    CriticalSection m_criticalsection;
#endif
};

/*
 * A MultipleTickHandler clocks all the AsyncLinks it is given.
 */

class MultipleTickHandler: public TickHandler {
public:
    MultipleTickHandler();
    void addLink(AsyncLink* link);
    // TickHandler
    void tick() override;
private:
    std::vector<AsyncLink*> m_links;
};

#endif // _GPIOASYNCLINK_H
