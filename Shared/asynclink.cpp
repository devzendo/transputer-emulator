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
#include "log.h"

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



AsyncLink::AsyncLink(int linkNo, bool isServer, TxRxPin& tx_rx_pin) :
    Link(linkNo, isServer), m_pin(tx_rx_pin) {
    logDebugF("Constructing async link %d for %s", myLinkNo, isServer ? "server" : "cpu client");
    myWriteSequence = myReadSequence = 0;
}

void AsyncLink::initialise() {
    
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

void AsyncLink::poll() {
	// no-op
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



DataAckSender::DataAckSender(TxRxPin& tx_rx_pin) : m_pin(tx_rx_pin), m_sampleCount(0), m_bits(0), m_data(0) {
    // TODO mutex {
    m_state = DataAckSenderState::IDLE;
    // TODO }
}

DataAckSenderState DataAckSender::state() const {
    // TODO mutex {
    return m_state;
    // TODO }
}

void DataAckSender::sendAck() {
    // TODO if m_state != IDLE throw up
    // TODO mutex {
    m_sampleCount = 0;
    m_bits = 2;
    m_data = 0x01;
    m_state = DataAckSenderState::SENDING;
    // TODO }
}

void DataAckSender::sendData(const BYTE8 byte) {
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
    m_state = DataAckSenderState::SENDING;
    // TODO }
}

void DataAckSender::clock() {
    //logDebugF("clock > %d sample count %d bits %d data 0x%04X", m_state, m_sampleCount, m_bits, m_data);
    // TODO mutex {
    switch (m_state) {
        case DataAckSenderState::IDLE:
            break;
        case DataAckSenderState::SENDING:
            // Send the least significant bit of m_data.
            const bool one = m_data & 0x0001;
            m_pin.setTx(one);
            m_sampleCount ++;
            if (m_sampleCount == 16) {
                m_sampleCount = 0;
                m_bits--;
                m_data >>= 1;
                if (m_bits == 0) {
                    m_state = DataAckSenderState::IDLE;
                }
            }
            break;
    }
    // TODO }
    //logDebugF("clock < %d sample count %d bits %d data 0x%04X", m_state, m_sampleCount, m_bits, m_data);
}

int DataAckSender::_queueLength() const {
    return m_bits;
}

WORD16 DataAckSender::_data() const {
    return m_data;
}



DataAckReceiver::DataAckReceiver(TxRxPin& tx_rx_pin) : m_pin(tx_rx_pin) {
    m_state = DataAckReceiverState::IDLE;
}

DataAckReceiverState DataAckReceiver::state() const {
    return m_state;
}

void DataAckReceiver::bitStateReceived(const bool state) {
    switch (m_state) {
        case DataAckReceiverState::IDLE:
            if (state) {
                m_state = DataAckReceiverState::START_BIT_2;
            }
            break;
        case DataAckReceiverState::START_BIT_2:
            if (state) {
                // TODO -> DATA
            } else {
                if (m_ack_receiver != nullptr) {
                    m_ack_receiver->ackReceived();
                }
                m_state = DataAckReceiverState::IDLE;
            }
            break;
        case DataAckReceiverState::DATA:
            break;
        case DataAckReceiverState::STOP_BIT:
            break;
    }
}

// Register the ack listener
void DataAckReceiver::registerAckReceiver(AckReceiver& ackReceiver) const {
    m_ack_receiver = &ackReceiver;
}

// Unregister the ack listener
void DataAckReceiver::unregisterAckReceiver() const {
    m_ack_receiver = nullptr;
}


