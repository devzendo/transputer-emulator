//------------------------------------------------------------------------------
//
// File        : testasynclink.cpp
// Description : Tests for asynchronous link abstraction
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 26/09/2025
//
// (C) 2005-2025 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <vector>

#include "gtest/gtest.h"
#include "link.h"
#include "asynclink.h"
#include "log.h"

class CrosswiredTxRxPinPair {
    bool aPin = false;
    bool bPin = false;

    class CrosswiredPin: TxRxPin {
    public:
        CrosswiredPin(const char *side, bool *rxstate, bool *txstate) : m_rxstate(rxstate), m_txstate(txstate) {
            m_side = *side;
        }

        ~CrosswiredPin() override = default;

        bool getRx() override {
            bool retval = (bool) *m_rxstate;
            logDebugF("RX %c pin state is %d", m_side, retval);
            return retval;
        }

        void setTx(const bool state) override {
            logDebugF("Setting TX %c pin state to %d", m_side, state);
            *m_txstate = state;
        }
    private:
        bool *m_rxstate;
        bool *m_txstate;
        char m_side = ' ';
    };

    CrosswiredPin m_aPinEnd;
    CrosswiredPin m_bPinEnd;
public:
    CrosswiredTxRxPinPair() :
        m_aPinEnd(CrosswiredPin("A", &aPin, &bPin)),
        m_bPinEnd(CrosswiredPin("B", &bPin, &aPin)) {
    }

    TxRxPin & pairA() { return reinterpret_cast<TxRxPin&>(m_aPinEnd); };

    TxRxPin & pairB() { return reinterpret_cast<TxRxPin&>(m_bPinEnd); };

    ~CrosswiredTxRxPinPair() {
    }
};


class AsyncLinkTest : public ::testing::Test {
protected:

    void SetUp() override {
        setLogLevel(LOGLEVEL_DEBUG);
        logDebug("SetUp start");

        TxRxPin &pairA = pair.pairA();
        TxRxPin &pairB = pair.pairB();

        logDebug("Creating Link A");
        linkA = new AsyncLink(0, false, pairA);
        linkA->setDebug(true);
        logDebug("Initialising Link A");
        linkA->initialise();

        logDebug("Creating Link B");
        linkB = new AsyncLink(0, false, pairB);
        linkB->setDebug(true);
        logDebug("Initialising Link B");
        linkB->initialise();

        logDebug("Setup complete");
        logFlush();
    }

    void TearDown() override {
        logDebug("TearDown start");
        if (linkA != nullptr) {
            logDebug("Resetting Link A");
            linkA->resetLink();
	        delete linkA;
        }
        if (linkB != nullptr) {
            logDebug("Resetting Link B");
            linkB->resetLink();
	        delete linkB;
        }
        logDebug("TearDown complete");
        logFlush();
    }

    CrosswiredTxRxPinPair pair;
    AsyncLink *linkA = nullptr;
    AsyncLink *linkB = nullptr;
};

/*
TEST_F(AsyncLinkTest, WriteAndReadByte) {
    linkA->writeByte(16);
    EXPECT_EQ(linkB->readByte(), 16);
}

TEST_F(AsyncLinkTest, WriteAndReadBytes) {
    BYTE8 writeBuf[4] = { 0xff, 0x7f, 0x60, 0x21 };
    int bytesWritten = linkA->writeBytes(writeBuf, 4);
    EXPECT_EQ(bytesWritten, 4);

    BYTE8 readBuf[4];
    int bytesRead = linkB->readBytes(readBuf, 4);
    EXPECT_EQ(bytesRead, 4);

    EXPECT_EQ(readBuf[0], 0xff);
    EXPECT_EQ(readBuf[1], 0x7f);
    EXPECT_EQ(readBuf[2], 0x60);
    EXPECT_EQ(readBuf[3], 0x21);
}
*/

class TxRxPinTest : public ::testing::Test {
protected:

    void SetUp() override {
        setLogLevel(LOGLEVEL_DEBUG);
        logDebug("SetUp start");

        logDebug("Setup complete");
        logFlush();
    }

    void TearDown() override {
        logDebug("TearDown start");
        logDebug("TearDown complete");
        logFlush();
    }

    CrosswiredTxRxPinPair pair;
};


TEST_F(TxRxPinTest, CrosswiredTxRxPinPairExecise) {
    TxRxPin &pairA = pair.pairA();
    TxRxPin &pairB = pair.pairB();

    // Initial state
    logInfo("Checking initial state");
    EXPECT_EQ(pairA.getRx(), false);
    EXPECT_EQ(pairB.getRx(), false);

    logInfo("Starting pin mutations");
    // Set A, see it at B
    pairA.setTx(true);
    EXPECT_EQ(pairB.getRx(), true);
    // No change in A
    EXPECT_EQ(pairA.getRx(), false);

    // Set B, see it at A
    pairB.setTx(true);
    EXPECT_EQ(pairA.getRx(), true);
    // No change in B
    EXPECT_EQ(pairB.getRx(), true);

    // Clear A, see it at B
    pairA.setTx(false);
    EXPECT_EQ(pairB.getRx(), false);
    // No change in A
    EXPECT_EQ(pairA.getRx(), true);

    // Clear B, see it at A
    pairB.setTx(false);
    EXPECT_EQ(pairA.getRx(), false);
    // No change in B
    EXPECT_EQ(pairB.getRx(), false);
}

class ClockingPinTracer {
public:
    ClockingPinTracer(TxRxPin& tx_rx_pin) : m_pin(tx_rx_pin) {
    }

    void clock() {
        m_trace.push_back(m_pin.getRx());
    }

    std::vector<bool> heard() {
        return m_trace;
    }

private:
    TxRxPin & m_pin;
    std::vector<bool> m_trace;
};

std::vector<bool> generate_sample_vector_from_bits(const int *bits, const int len) {
    std::vector<bool> out;
    logDebugF("generating sample vector from %d bits", len);
    for (int i=0; i<len; i++) {
        const bool one = (bits[i] == 1);
        logDebugF("expecting %d", one);
        for (int b=0; b<16; b++) {
            out.push_back(one);
        }
    }
    return out;
}

std::string send_input_get_output(TxRxPin &input_pin, const std::string &input_samples, TxRxPin &output_pin) {
    std::string out;
    for (const char input_sample : input_samples) {
        input_pin.setTx(input_sample == '1');
        out.push_back(output_pin.getRx() ? '1' : '0');
    }
    return out;
}

TEST_F(TxRxPinTest, OversampledTxRxPinStraightThroughTx) {
    TxRxPin &pairA = pair.pairA();
    OversampledTxRxPin o_pin(pairA); // can Rx the majority-voted signal via o_pin
    TxRxPin &pairB = pair.pairB();

    EXPECT_EQ(pairB.getRx(), false);

    o_pin.setTx(true);
    EXPECT_EQ(pairB.getRx(), true);

    o_pin.setTx(false);
    EXPECT_EQ(pairB.getRx(), false);
}

TEST_F(TxRxPinTest, OversampledTxRxPinAllFalseInput) {
    TxRxPin &pairA = pair.pairA();
    OversampledTxRxPin o_pin(pairA); // can Rx the majority-voted signal via o_pin
    TxRxPin &pairB = pair.pairB();

    const std::string input_bits = "00000000000000000000000000000000";
    const std::string received = send_input_get_output(pairB, input_bits, reinterpret_cast<TxRxPin&>(o_pin));

    EXPECT_EQ(received, "00000000000000000000000000000000");
    EXPECT_EQ(o_pin._resync_in_samples(), 0);
}


TEST_F(TxRxPinTest, AckCanBeSent) {
    TxRxPin & pairA = pair.pairA();
    TxRxPin & pairB = pair.pairB();
    ClockingPinTracer trace(pairB);
    DataAckSender sender(pairA);
    EXPECT_EQ(sender.state(), IDLE);

    sender.sendAck(); // Needs clocking to send the signals out. 16 clock pulses/bit.

    // An ack is a 1 followed by a 0.
    const int expected_bits = 2;
    const int expected_samples = expected_bits * 16;
    EXPECT_EQ(sender.state(), SENDING);
    EXPECT_EQ(sender._queueLength(), 2);
    EXPECT_EQ(sender._data(), 0x0001);
    for (int i=0; i<expected_samples; i++) {
        EXPECT_EQ(sender.state(), SENDING);

        sender.clock();
        trace.clock();
    }
    EXPECT_EQ(sender.state(), IDLE);
    EXPECT_EQ(sender._queueLength(), 0);
    const std::vector<bool> heard = trace.heard();
    EXPECT_EQ(heard.size(), expected_samples);
    const int expected_ack_bits[expected_bits] = { 1, 0 };
    const std::vector<bool> expected = generate_sample_vector_from_bits(expected_ack_bits, expected_bits);
    EXPECT_EQ(heard, expected);
}

TEST_F(TxRxPinTest, DataCanBeSent) {
    TxRxPin & pairA = pair.pairA();
    TxRxPin & pairB = pair.pairB();
    ClockingPinTracer trace(pairB);
    DataAckSender sender(pairA);
    EXPECT_EQ(sender.state(), IDLE);

    sender.sendData(0xC9); // Needs clocking to send the signals out. 16 clock pulses/bit.

    // Data is two start bits (1), the data bits, then a stop bit (0).
    const int expected_bits = 11;
    const int expected_samples = expected_bits * 16;
    EXPECT_EQ(sender.state(), SENDING);
    EXPECT_EQ(sender._queueLength(), expected_bits);
    EXPECT_EQ(sender._data(), 0x0327);
    logDebug("Data to be clocked out looks right with framing");
    for (int i=0; i<expected_samples; i++) {
        EXPECT_EQ(sender.state(), SENDING);

        sender.clock();
        trace.clock();
    }
    EXPECT_EQ(sender.state(), IDLE);
    EXPECT_EQ(sender._queueLength(), 0);
    const std::vector<bool> heard = trace.heard();
    EXPECT_EQ(heard.size(), expected_samples);   //       vvv note LSB first vvv
    const int expected_ack_bits[expected_bits] = { 1, 1,  1, 0, 0, 1, 0, 0, 1, 1,  0 };
    const std::vector<bool> expected = generate_sample_vector_from_bits(expected_ack_bits, expected_bits);
    EXPECT_EQ(heard, expected);
}

// TODO how to signal you can't ack or send data in a non-idle state