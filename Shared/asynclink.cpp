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

void AsyncLink::initialise(void) {
    
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

void AsyncLink::resetLink(void) {
    // TODO
}

int AsyncLink::getLinkType() {
    return LinkType_Async;
}

void AsyncLink::poll(void) {
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



DataAckSender::DataAckSender(TxRxPin& tx_rx_pin) : m_pin(tx_rx_pin) {
    // TODO mutex {
    m_state = IDLE;
    // TODO }
}

DataAckSenderState DataAckSender::state() {
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
    m_state = SENDING;
    // TODO }
}

void DataAckSender::sendData(BYTE8 byte) {
    // TODO if m_state != IDLE throw up
    // TODO mutex {
    BYTE8 origByte = byte;
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
    m_state = SENDING;
    // TODO }
}

void DataAckSender::clock() {
    //logDebugF("clock > %d sample count %d bits %d data 0x%04X", m_state, m_sampleCount, m_bits, m_data);
    // TODO mutex {
    switch (m_state) {
    case IDLE:
        break;
    case SENDING:
        // Send the least significant bit of m_data.
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

int DataAckSender::_queueLength() {
    return m_bits;
}

WORD16 DataAckSender::_data() {
    return m_data;
}
