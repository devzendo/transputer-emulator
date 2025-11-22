//------------------------------------------------------------------------------
//
// File        : asynclink.cpp
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

#include <exception>
#include "asynclink.h"
#include "misc.h"
#include "log.h"

#ifdef PICO
#include "pico/stdlib.h"
#endif

/*
 * A TxRxPin decorator that performs majority voting to provide solid bit-long true/false values for all the samples
 * in a bit.
 *
 * setTx() is passed straight through to the underlying TxRxPin - this class is used for its function in reception.
 *
 * Samples 7, 8 and 9 of a bit (after a sync point) form the majority vote.
 *
 * Rising edge is used to detect the start of a bit. Needs knowledge of ack and data frame lengths to know when an ack
 * or data frame has finished, so it can re-sync its start-of-bit detection. There could be a delay between Data and a
 * following Ack, or there may be no delay at all. The start of bit detection must take place on every Data or Ack.
 *
 * The majority vote result is not known until bit 9, but this knowledge is not visible via getRx() until the end of the
 * bit - this is BECAUSE DAMMIT WHY WAS THIS?
 *
 * The majority vote result is not known until bit 9, when this knowledge will be made available via getRx().
 *
 * Consider: whether majority voting should be used to detect the initial rising edge, rather than syncing on the first
 * rising edge sample (which could be noise).
*/
OversampledTxRxPin::OversampledTxRxPin(TxRxPin& tx_rx_pin) :
    m_pin(tx_rx_pin), m_resync_in_samples(0), m_sample_index(0), m_data_bits_length(0),
    m_data_samples(0), m_data_bits(0),
    m_previous_rx(false), m_latched_output_rx(false),
    m_rx_bit_receiver(nullptr) {
}

bool OversampledTxRxPin::getRx() {
    const bool rx = m_pin.getRx();

    // Majority vote taken on the rightmost 3 of m_data. 011, 101, 110 = 3,5,6
    m_data_samples <<= 1;
    m_data_samples |= rx;
    logDebugF("rx %d m_data_samples 0b%s sample index %d resync %d", rx, word_to_binary(m_data_samples), m_sample_index,
        m_resync_in_samples);

    // Detect rising edge, so we can count samples for majority vote detection
    const bool rising_edge = !m_previous_rx && rx;
    if (m_resync_in_samples == 0 && rising_edge) {
        m_data_bits = 0;
        m_data_bits_length = 0;
        // Don't know what the first two bits are yet (we'll collect them in m_data_bits), but the shortest resync could
        // be after an ack, so be pessimistic and sync again after a potential ack.
        m_resync_in_samples = 31 + 1; // +1 since we'll decrement below
        logDebug("Synchronising majority vote detection on rising edge; setting resync at end of possible ack");
        m_sample_index = 0;
    }

    if (m_sample_index == 8) {
        WORD16 majority_samples = m_data_samples & 0x0007;
        m_latched_output_rx = (majority_samples == 3 || majority_samples == 5 || majority_samples == 6 || majority_samples == 7);
        logDebugF("majority vote is %d = 0b%s : latched output %d", majority_samples,
            word_to_binary(majority_samples), m_latched_output_rx);
        m_data_bits <<= 1;
        m_data_bits |= m_latched_output_rx;
        m_data_bits_length++;
        logDebugF("m_data_bits 0b%s length %d", word_to_binary(m_data_bits), m_data_bits_length);
        // Got 2 bits? Can set resync correctly now - we know whether it's ack/data/unknown.
        if (m_data_bits_length == 2) {
            switch (m_data_bits) {
                case 0x0003:
                    // data (full frame 11 bits [ 1 1 x x x x x x x x 0 ], so 16x11=176 samples)
                    // but we're at the end of the majority vote bits of the second lot of 16 samples, so we've seen 25 samples,
                    // so take those off: 176-25=151.
                    // 0123456789ABCDEF0123456789ABCDEF
                    // xxxxxx|||xxxxxxxyyyyyy|||yyyyyyy
                    m_resync_in_samples = 151 + 1; // +1 since we'll decrement below;
                    logDebug("Data detected; setting resync at end of data");
                    break;
                default:
                    logInfo("Start of frame was not ack or data");
                    break;
            }
        }
        // Notify the bit receiver, if there is one.
        if (m_rx_bit_receiver != nullptr) {
            m_rx_bit_receiver->bitStateReceived(m_latched_output_rx);
        }
    }

    m_sample_index++;

    if (m_sample_index == 16) {
        logDebug("Resetting sample index");
        m_sample_index = 0;
    }

    if (m_resync_in_samples > 0) {
        m_resync_in_samples--;
    }
    m_previous_rx = rx; // For rising edge detection.
    logDebugF("rx input %d output %d, resync in %d samples", rx, m_latched_output_rx, m_resync_in_samples);
    return m_latched_output_rx;
}

// setTx passes its value straight through to the underlying pin.
void OversampledTxRxPin::setTx(const bool state) {
    m_pin.setTx(state);
}

// Register the receiver listener
void OversampledTxRxPin::registerRxBitReceiver(RxBitReceiver& rxBitReceiver) const {
    m_rx_bit_receiver = &rxBitReceiver;
}

// Unregister the receiver listener
void OversampledTxRxPin::unregisterRxBitReceiver() const {
     m_rx_bit_receiver = nullptr;
}

int OversampledTxRxPin::_resync_in_samples() const {
    return m_resync_in_samples;
}

#ifdef PICO
// TODO this is global, and prevents multiple cores - but would you just have one clock
// for the whole board?
// Free function for the timer callback.
static bool timerCallback(repeating_timer_t *rt) {
    AsyncLinkClock* clock = static_cast<AsyncLinkClock*>(rt->user_data);
    clock->operator()();
    return clock->is_running(); // true to continue repeating
}
#endif


AsyncLinkClock::AsyncLinkClock(uint clockGPIOPin, TickHandler& tickHandler) :
    m_clockGPIOPin(clockGPIOPin), m_tick_handler(tickHandler) {
    logDebugF("Creating AsyncLinkClock with pin %d", m_clockGPIOPin);
#ifdef PICO
    // Initialise the clock diagnostic pin...
    gpio_init(m_clockGPIOPin);
    gpio_set_dir(m_clockGPIOPin, GPIO_OUT);
    gpio_set_drive_strength(m_clockGPIOPin, GPIO_DRIVE_STRENGTH_2MA);
    gpio_put(m_clockGPIOPin, false);
#endif
}

void AsyncLinkClock::start() {
    logDebug("Starting AsyncLinkClock; starting TickHandler");
    m_tick_handler.start();
    logDebug("TickHandler started");
    // The tick handler (base class) will prevent this repeating timer being set up if it
    // has been stopped.
#ifdef PICO
    // Set up the timer...
    // Negative delay so means we will call repeating_timer_callback, and call it again
    // at the next tick interval regardless of how long the callback took to execute.
    add_repeating_timer_us(-LINK_CLOCK_TICK_INTERVAL_US, &timerCallback, this, &m_timer);
#endif
#ifdef DESKTOP
    logDebug("Starting thread");
    m_thread = new std::thread([this] {
        while (is_running()) {
            this->operator()();
            std::this_thread::sleep_for(std::chrono::microseconds(LINK_CLOCK_TICK_INTERVAL_US));
        }
    });

#endif
    logDebug("Started AsyncLinkClock");
}

bool AsyncLinkClock::is_running() {
    return m_tick_handler.is_running();
}

void AsyncLinkClock::stop() {
    logDebug("Stopping AsyncLinkClock");
    logDebug("Stopping TickHandler");
    m_tick_handler.stop();
#ifdef DESKTOP
    if (m_thread != nullptr) {
        logDebug("Joining thread");
        m_thread->join();
        logDebug("Thread joined");
    }
#endif
    logDebug("Stopped AsyncLinkClock");
}

void AsyncLinkClock::operator()() const {
    // logDebug("Tick!");
#ifdef PICO
    gpio_put(m_clockGPIOPin, true);
#endif
    m_tick_handler.tick();
#ifdef PICO
    gpio_put(m_clockGPIOPin, false);
#endif
}


AsyncLinkClock::~AsyncLinkClock() {
    logDebug("Destroying AsyncLinkClock");
}


// Only one byte needed to buffer in the receiving link (DataAckReceiver).
// Ack can be sent as soon as reception of a data byte starts, if there's room to buffer another - need state to store
// whether an ack needs sending (start bits of data received). Need state to indicate whether the data buffer is full
// that gets cleared when it's read.
// Receiver:
// If rising edge of start bits received:
//   if buffer empty:
//     sender.sendAck()
//   else
//     ack required = true
//
// Data buffer clear:
//   buffer = 0x00
//   data available = false
//   if ack required:
//     ack required = false
//     sender.sendAck()

// Sender:
//   Initial tx pin state = 0

// Needs to notify clients:
// For a sender:
// * That an ack has been received - the emulator only reschedules the sending process when the ack for the final byte
//   has been received.

/*
 * Status word bits.
 * 15       | 14       | 13        | 12        | 11        | 10        | 9         | 8
 * FRAMING  | OVERRUN  | READ DATA | READY TO  | DATA SENT | ......... | ......... | ......... |
 *          |          | AVAILABLE | SEND      | NOT ACKED |           |           |           |
 *          |          |           |           | (TIMEOUT) |           |           |           |
 * ---------------------------------------------------------------------------------------------
 * 7        | 6        | 5         | 4         | 3         | 2         | 1         | 0
 * DATA RECEIVED IF READ DATA AVAILABLE (BIT 13) IS TRUE
 */
const WORD16 ST_FRAMING = 0x8000;
const WORD16 ST_OVERRUN = 0x4000;
const WORD16 ST_READ_DATA_AVAILABLE = 0x2000;
const WORD16 ST_READY_TO_SEND = 0x1000;
const WORD16 ST_DATA_SENT_NOT_ACKED = 0x0800;
const WORD16 ST_DATA_MASK = 0x00FF;

AsyncLink::AsyncLink(int linkNo, bool isServer, TxRxPin& tx_rx_pin) :
    Link(linkNo, isServer), m_pin(tx_rx_pin), m_status_word(0) {
    logDebugF("Constructing async link %d for %s", myLinkNo, isServer ? "server" : "cpu client");
    myWriteSequence = myReadSequence = 0;

    m_o_pin = new OversampledTxRxPin(tx_rx_pin);
    m_receiver = new DataAckReceiver();
    m_receiver->registerReceiverToLink(*this); // dereference this to get a reference.
    m_o_pin->registerRxBitReceiver(*m_receiver);
    // The sender can use the TxRxPin directly - it could use the OversampledTxRxPin
    // which would pass setTx straight through, but direct is quicker.
    m_sender = new DataAckSender(tx_rx_pin);
    m_receiver->registerReceiverToSender(*m_sender);
    m_sender->registerSenderToLink(*this); // dereference this to get a reference.
}

void AsyncLink::initialise() {
    setReadyToSend();
}

AsyncLink::~AsyncLink() {
    logDebugF("Destroying async link %d", myLinkNo);
}

BYTE8 AsyncLink::readByte() {
    if (bDebug) {
        logDebugF("Link %d R #%08X 00 (.)", myLinkNo, myReadSequence++);
    }
    return 0;
}

void AsyncLink::writeByte(BYTE8 buf) {
    if (bDebug) {
        logDebugF("Link %d W #%08X 00 (.)", myLinkNo, myWriteSequence++);
    }
}

void AsyncLink::resetLink() {
    // TODO
}

int AsyncLink::getLinkType() {
    return LinkType_Async;
}

void AsyncLink::clock() {
    logDebugF("Clocking link %d", myLinkNo);
	m_sender->clock(); // Will start clocking data out, if not idle.
    m_o_pin->getRx(); // Will call the majority vote callback with the input bit.
}

bool AsyncLink::writeByteAsync(unsigned char b) {
    return m_sender->sendData(0xC9);
}


// SenderToLink
bool AsyncLink::queryReadyToSend() {
    MUTEX
    return m_status_word & ST_READY_TO_SEND;
}

void AsyncLink::setReadyToSend() {
    MUTEX
    m_status_word |= ST_READY_TO_SEND;
}

void AsyncLink::clearReadyToSend() {
    MUTEX
    m_status_word &= ~ST_READY_TO_SEND;
}

void AsyncLink::setTimeoutError() {
    // TODO
}

// ReceiverToLink
void AsyncLink::framingError() {
    // TODO
}

void AsyncLink::overrunError() {
    // TODO
}

void AsyncLink::dataReceived(BYTE8 data) {
    // TODO
}

bool AsyncLink::queryReadDataAvailable() {
    // TODO
    return false;
}

void AsyncLink::clearReadDataAvailable() {
    // TODO
}


/*
 * A MultipleTickHandler clocks all the Links it is given.
 */
MultipleTickHandler::MultipleTickHandler() : TickHandler() {

}

void MultipleTickHandler::addLink(Link* link) {
    m_links.push_back(link);
}


// TickHandler
void MultipleTickHandler::tick() {
    logDebug("Tick - clock the Links");
    for (Link* link: m_links) {
        logDebugF("Tick - link at 0x%x", link);
        link->clock();
    }
}



/*
 * Using oversampling: The clock pulse triggers 16 times the bit frequency, giving 16 samples per bit.
 * |         1111111|
 * |1234567890123456|
 *        xxx
 * Only samples 7, 8 and 9 are read, and a majority vote taken to determine the actual value of the
 * transmitted bit.
 *
 * Note: Patrick H. Stakem's book states that 'The incoming data was sampled at five times the bit
 * frequency.'
 */

const char* DataAckSenderStateToString(const DataAckSenderState s) noexcept
{
    switch (s) {
        case DataAckSenderState::IDLE: return "IDLE";
        case DataAckSenderState::SENDING_ACK: return "SENDING_ACK";
        case DataAckSenderState::SENDING_DATA: return "SENDING_DATA";
        case DataAckSenderState::ACK_TIMEOUT: return "ACK_TIMEOUT";
    }
    return "UNKNOWN";
}


DataAckSender::DataAckSender(TxRxPin& tx_rx_pin) : m_pin(tx_rx_pin), m_send_ack(false), m_ack_rxed(false), m_sampleCount(0),
    m_bits(0), m_data(0), m_data_enqueued(false), m_data_enqueued_buffer(0x00) {

    // TODO mutex {
    m_state = DataAckSenderState::IDLE;
    // TODO }
}

DataAckSenderState DataAckSender::state() const {
    // TODO mutex {
    return m_state;
    // TODO }
}

// ReceiverToSender
void DataAckSender::ackReceived() {
    switch (m_state) {
        case DataAckSenderState::IDLE:
            logWarn("Ack received in IDLE state");
            break;
        case DataAckSenderState::SENDING_DATA:
            logDebug("Data being sent has been acked");
            m_ack_rxed = true;
            break;
        case DataAckSenderState::SENDING_ACK:
            logDebug("Ack being sent has been acked");
            m_ack_rxed = true;
            break;
        case DataAckSenderState::ACK_TIMEOUT:
            logDebug("Ack being sent has been acked (timeout)");
            m_ack_rxed = true;
            break;
        default:
            logDebug("Ack received");
            break;
    }
}

// ReceiverToSender
void DataAckSender::sendAck() {
    logDebug("The sending of an ack has been requested");
    switch (m_state) {
        case DataAckSenderState::IDLE:
            m_sampleCount = 0;
            m_bits = 2;
            m_data = 0x01;
            changeState(DataAckSenderState::SENDING_ACK);
            break;
        case DataAckSenderState::SENDING_DATA:
            m_send_ack = true;
            // The data/bits for the ack is set up (as above) when we detect m_send_ack in clock...
            break;
        default:
            break;
    }
}

bool DataAckSender::sendData(const BYTE8 byte) {
    // TODO mutex {
    switch (m_state) {
        case DataAckSenderState::IDLE:
            if (m_sender_to_link != nullptr && m_sender_to_link->queryReadyToSend()) {
                m_ack_rxed = false;
                m_sender_to_link->clearReadyToSend();
                sendDataInternal(byte);
                return true;
            }
            return false;
        case DataAckSenderState::SENDING_ACK:
            logDebugF("Enqueueing data to send 0b%s", byte_to_binary(byte));
            m_data_enqueued = true;
            m_data_enqueued_buffer = byte;
            m_ack_rxed = false;
            return true;
        case DataAckSenderState::SENDING_DATA:
            // DROP THROUGH
        default:
            logWarnF("Sending data in %s state", DataAckSenderStateToString(m_state));
            return false;
    }
    // TODO }
}

void DataAckSender::sendDataInternal(const BYTE8 byte) {
    // TODO if m_state != IDLE throw up
    // TODO mutex {
    const BYTE8 origByte = byte;
    m_sampleCount = 0;
    m_bits = 11;
    // Data is shifted out from the LSB of m_data. After the start bits (1 1), 'byte' is sent, starting with the
    // least significant bit of 'byte' then one stop bit (0).
    m_data = (byte << 2) | 0x0003; // A stop bit (implied), 'byte', then two start bits.
    logDebugF("sendData orig byte 0b%s", byte_to_binary(origByte));
    logDebugF("sendData data 0b%s", word_to_binary(m_data));
    // 1
    // 0 5 2 1
    // 2 1 5 2 6 3 1
    // 4 2 6 8 4 2 6 8 4 2 1
    // ---------------------
    // 0 b b b b b b b b 1 1  <-- m_data
    // 0 7 6 5 4 3 2 1 0 1 1
    // e.g. byte == C9
    // 0 1 1 0 0 1 0 0 1 1 1
    // 3     2       7
    changeState(DataAckSenderState::SENDING_DATA);
    // TODO }
}

void DataAckSender::clock() {
    //logDebugF("clock > %d sample count %d bits %d data 0x%04X", m_state, m_sampleCount, m_bits, m_data);
    // TODO mutex {
    switch (m_state) {
        case DataAckSenderState::IDLE:
            // Fall through
        case DataAckSenderState::ACK_TIMEOUT:
            break;
        case DataAckSenderState::SENDING_ACK:
        case DataAckSenderState::SENDING_DATA:
            // Send the least significant bit of m_data.
            const bool one = m_data & 0x0001;
            m_pin.setTx(one);
            m_sampleCount ++;
            if (m_sampleCount == 16) {
                m_sampleCount = 0;
                m_bits--;
                m_data >>= 1;
                if (m_bits == 0) {
                    logDebugF("End of transmission, state is %s, ack_rxed %d data_enqueued %d send_ack %d", DataAckSenderStateToString(m_state), m_ack_rxed, m_data_enqueued, m_send_ack);
                    if (m_state == DataAckSenderState::SENDING_ACK) {
                        if (m_data_enqueued) {
                            m_ack_rxed = false;
                            sendDataInternal(m_data_enqueued_buffer); // Will change to SENDING_DATA.
                            m_data_enqueued = false;
                            m_data_enqueued_buffer = 0x00;
                        } else {
                            if (m_send_ack) {
                                if (m_ack_rxed) {
                                    changeState(DataAckSenderState::IDLE);
                                } else {
                                    changeState(DataAckSenderState::ACK_TIMEOUT);
                                }
                            } else {
                                changeState(DataAckSenderState::IDLE);
                            }
                        }
                    } else { // SENDING_DATA
                        if (m_send_ack) {
                            m_sampleCount = 0;
                            m_bits = 2;
                            m_data = 0x01;
                            changeState(DataAckSenderState::SENDING_ACK);
                        } else {
                            if (m_ack_rxed) {
                                changeState(DataAckSenderState::IDLE);
                            } else {
                                changeState(DataAckSenderState::ACK_TIMEOUT);
                            }
                        }
                    }

                }
            }
            break;
    }
    // TODO }
    //logDebugF("clock < %d sample count %d bits %d data 0x%04X", m_state, m_sampleCount, m_bits, m_data);
}

// Register the SenderToLink
void DataAckSender::registerSenderToLink(SenderToLink& senderToLink) const {
    m_sender_to_link = &senderToLink;
    m_sender_to_link->setReadyToSend();
}

// Unregister the SenderToLink
void DataAckSender::unregisterSenderToLink() const {
    m_sender_to_link = nullptr;
}

void DataAckSender::changeState(const DataAckSenderState newState) {
    // State exit actions
    logDebugF("Exiting state %s", DataAckSenderStateToString(m_state));
    switch (m_state) {
        case DataAckSenderState::SENDING_ACK:
            m_send_ack = false;
            break;
        case DataAckSenderState::IDLE:
            // Fall through
        case DataAckSenderState::SENDING_DATA:
            // Fall through
        case DataAckSenderState::ACK_TIMEOUT:
            break;
    }

    // State entry actions
    logDebugF("Entering state %s", DataAckSenderStateToString(newState));
    m_state = newState;
    switch (newState) {
        case DataAckSenderState::IDLE:
            if (m_ack_rxed) {
                m_ack_rxed = false;
                if (m_sender_to_link != nullptr) {
                    m_sender_to_link->setReadyToSend();
                }
            }
            break;

        case DataAckSenderState::SENDING_ACK:
            if (m_sender_to_link != nullptr) {
                m_sender_to_link->setReadyToSend();
            }
            m_ack_rxed = false;
            break;
        case DataAckSenderState::SENDING_DATA:
            break;
        case DataAckSenderState::ACK_TIMEOUT:
            if (m_sender_to_link != nullptr) {
                m_sender_to_link->setTimeoutError();
            }
            break;
    }

}

int DataAckSender::_queueLength() const {
    return m_bits;
}

WORD16 DataAckSender::_data() const {
    return m_data;
}

bool DataAckSender::_send_ack() const {
    return m_send_ack;
}

bool DataAckSender::_ack_rxed() const {
    return m_ack_rxed;
}

bool DataAckSender::_data_enqueued() const {
    return m_data_enqueued;
}

BYTE8 DataAckSender::_data_enqueued_buffer() const {
    return m_data_enqueued_buffer;
}


constexpr const char* DataAckReceiverStateToString(const DataAckReceiverState s) noexcept
{
    switch (s)
    {
        case DataAckReceiverState::IDLE: return "IDLE";
        case DataAckReceiverState::START_BIT_2: return "START_BIT_2";
        case DataAckReceiverState::DATA: return "DATA";
        case DataAckReceiverState::DISCARD: return "DISCARD";
        case DataAckReceiverState::STOP_BIT: return "STOP_BIT";
    }
    return "UNKNOWN";
}

DataAckReceiver::DataAckReceiver() {
    m_state = DataAckReceiverState::IDLE;
    // These nonsense values are reset when data reception starts and are set to this, to ensure this
    // reset happens.
    m_bit_count = -1;
    m_buffer = 0x69;
}

DataAckReceiverState DataAckReceiver::state() const {
    return m_state;
}

void DataAckReceiver::bitStateReceived(const bool state) {
    switch (m_state) {
        case DataAckReceiverState::IDLE:
            if (state) {
                changeState(DataAckReceiverState::START_BIT_2);
            }
            break;
        case DataAckReceiverState::START_BIT_2:
            if (state) {
                m_bit_count = 0;
                m_buffer = 0x00;
                if (m_receiver_to_link != nullptr) {
                    if (m_receiver_to_link->queryReadDataAvailable()) {
                        m_receiver_to_link->overrunError();
                        changeState(DataAckReceiverState::DISCARD);
                    } else {
                        if (m_receiver_to_sender != nullptr) {
                            m_receiver_to_sender->sendAck();
                            changeState(DataAckReceiverState::DATA);
                        } else {
                            changeState(DataAckReceiverState::DISCARD);
                        }
                    }
                } else {
                    changeState(DataAckReceiverState::DISCARD);
                }
            } else {
                if (m_receiver_to_sender != nullptr) {
                    m_receiver_to_sender->ackReceived();
                }
                changeState(DataAckReceiverState::IDLE);
            }
            break;
        case DataAckReceiverState::DATA:
            if (m_bit_count < 8) {
                m_buffer <<= 1;
                m_buffer |= state;
                m_bit_count ++;
            }
            if (m_bit_count == 8) {
                changeState(DataAckReceiverState::STOP_BIT);
            }
            break;
        case DataAckReceiverState::DISCARD:
            if (m_bit_count < 9) {
                m_bit_count ++;
            }
            if (m_bit_count == 9) {
                changeState(DataAckReceiverState::IDLE);
            }
            break;
        case DataAckReceiverState::STOP_BIT:
            if (state) {
                if (m_receiver_to_link != nullptr) {
                    m_receiver_to_link->framingError();
                }
                changeState(DataAckReceiverState::IDLE);
            } else {
                if (m_receiver_to_link != nullptr) {
                    m_receiver_to_link->dataReceived(m_buffer);
                }
                changeState(DataAckReceiverState::IDLE);
            }
            break;
    }
}

// Register the ReceiverToSender
void DataAckReceiver::registerReceiverToSender(ReceiverToSender& receiverToSender) const {
    m_receiver_to_sender = &receiverToSender;
}

// Unregister the ReceiverToSender
void DataAckReceiver::unregisterReceiverToSender() const {
    m_receiver_to_sender = nullptr;
}

// Register the ReceiverToLink
void DataAckReceiver::registerReceiverToLink(ReceiverToLink& receiverToLink) const {
    m_receiver_to_link = &receiverToLink;
}

// Unregister the ReceiverToLink
void DataAckReceiver::unregisterReceiverToLink() const {
    m_receiver_to_link = nullptr;
}

void DataAckReceiver::changeState(const DataAckReceiverState newState) {
    logDebugF("Changing state from %s to %s", DataAckReceiverStateToString(m_state), DataAckReceiverStateToString(newState));
    m_state = newState;
}

int DataAckReceiver::_bit_count() const {
    return m_bit_count;
}

BYTE8 DataAckReceiver::_buffer() const {
    return m_buffer;
}



