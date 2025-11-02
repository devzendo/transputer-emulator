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
            //logDebugF("RX %c pin state is %d", m_side, retval);
            return retval;
        }

        void setTx(const bool state) override {
            //logDebugF("Setting TX %c pin state to %d", m_side, state);
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

    ~CrosswiredTxRxPinPair() = default;
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
    explicit ClockingPinTracer(TxRxPin& tx_rx_pin) : m_pin(tx_rx_pin) {
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

class OversampledTxRxPinTest : public ::testing::Test, RxBitReceiver {
public:
    OversampledTxRxPinTest() : m_pairA(m_pair.pairA()), m_pairB(m_pair.pairB()),
        m_o_pin(m_pairA) {
        m_o_pin.registerRxBitReceiver(*this); // dereference this to get a reference
    }
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

    void bitStateReceived(const bool state) override {
        logDebugF("Received bit: %d", state);
        m_received_bits.push_back(state ? '1' : '0');
    }

    std::string send_input_get_output(const std::string &input_samples) {
        std::string out;
        for (const char input_sample : input_samples) {
            m_pairB.setTx(input_sample == '1');
            out.push_back(m_o_pin.getRx() ? '1' : '0');
        }
        return out;
    }

    CrosswiredTxRxPinPair m_pair;
    TxRxPin &m_pairA;
    TxRxPin &m_pairB;

    OversampledTxRxPin m_o_pin;
    // Tx straight through; can Rx the majority-voted signal via o_pin
    // can Rx the majority-voted signal via o_pin

    // Bit states received by the RxBitReceiver implementation append to this
    // string as ASCII 1 / 0.
    std::string m_received_bits;
};

TEST_F(OversampledTxRxPinTest, StraightThroughTx) {
    EXPECT_EQ(m_pairB.getRx(), false);

    m_o_pin.setTx(true);
    EXPECT_EQ(m_pairB.getRx(), true);

    m_o_pin.setTx(false);
    EXPECT_EQ(m_pairB.getRx(), false);
}

TEST_F(OversampledTxRxPinTest, AllFalseInput) {
    //                                 0123456789ABCDEF0123456789ABCDEF
    const std::string input_samples = "00000000000000000000000000000000";
    const std::string received = send_input_get_output(input_samples);

    //                                 0123456789ABCDEF0123456789ABCDEF
    EXPECT_EQ(received,               "00000000000000000000000000000000");
    EXPECT_EQ(m_o_pin._resync_in_samples(), 0); // No sync pulse received, resync at next sample (0).
}

TEST_F(OversampledTxRxPinTest, SyncPulseDetection) {
    const std::string input_samples = "1";
    const std::string received = send_input_get_output(input_samples);

    EXPECT_EQ(received,               "0"); // there's a 9-bit delay before we'll see the majority voted bit - but not enough input data here.
    EXPECT_EQ(m_o_pin._resync_in_samples(), 31); // don't yet know the next two bits so we can determine the frame length,
    // so resync at end of a potential ack, in 31 samples time (2 bits - 1 sample)
}

TEST_F(OversampledTxRxPinTest, RisingEdgeAfterSyncPulseDetectionDoesNotResetResync) {
    const std::string input_samples = "101";
    const std::string received = send_input_get_output(input_samples);

    EXPECT_EQ(received,               "000"); // there's a 9-bit delay before we'll see the majority voted bit - but not enough input data here.
    EXPECT_EQ(m_o_pin._resync_in_samples(), 29); // don't yet know the next two bits so we can determine the frame length,
    // so resync at end of a potential ack, in 29 samples time (2 bits (32 samples) - 3 samples)
}

TEST_F(OversampledTxRxPinTest, SyncPulseInputZero) {
    //                                 0123456789ABCDEF
    const std::string input_samples = "1000000000000000";
    const std::string received = send_input_get_output(input_samples);

    //                                 0123456789ABCDEF
    EXPECT_EQ(received,               "0000000000000000");
    EXPECT_EQ(m_o_pin._resync_in_samples(), 16); // two bits (only got one so far) were not 1,1 (data) or 1,0 (ack) -
    // so resync at start of a potential ack

    // finish the ack...
    send_input_get_output("0000000000000000");
    EXPECT_EQ(m_o_pin._resync_in_samples(), 0);
    // another ack...
    send_input_get_output("10000000000000000000000000000000");
    EXPECT_EQ(m_o_pin._resync_in_samples(), 0);
}

// Bits at start of a frame:
// Zero Zero - invalid
// Zero One - invalid
// One Zero - ack
// One One - data

TEST_F(OversampledTxRxPinTest, SyncPulseInputZeroZero) {
    //                                 0123456789ABCDEF0123456789ABCDEF
    //                                 xxxxxx|||xxxxxxxyyyyyy|||yyyyyyy
    const std::string input_samples = "10000000000000000000000000000000000000000";
    const std::string received = send_input_get_output(input_samples);

    //                                 0123456789ABCDEF0123456789ABCDEF
    //                                          xxxxxxxxxxxxxxxxyyyyyyyyyyyyyyyy
    EXPECT_EQ(received,               "00000000000000000000000000000000000000000");
    EXPECT_EQ(m_o_pin._resync_in_samples(), 0); // two bits were not 1,1 (data) or 1,0 (ack) - so resync at next sample (0)
}

TEST_F(OversampledTxRxPinTest, SyncPulseInputZeroOne) {
    //                                 0123456789ABCDEF0123456789ABCDEF
    //                                 xxxxxx|||xxxxxxxyyyyyy|||yyyyyyy
    const std::string input_samples = "1000000000000000000000111000000000000000";
    const std::string received = send_input_get_output(input_samples);

    //                                 0123456789ABCDEF0123456789ABCDEF
    //                                         xxxxxxxxxxxxxxxxyyyyyyyyyyyyyyyy
    EXPECT_EQ(received,               "0000000000000000000000001111111111111111");
    EXPECT_EQ(m_o_pin._resync_in_samples(), 0); // two bits were not 1,1 (data) or 1,0 (ack) - so resync at next sample (0)
}

// One Zero is an ack, so test this with the majority detection cases...

void expect_ack(const std::string& received, int resync_in_samples, const std::string& received_bits) {
    //                                 0123456789ABCDEF0123456789ABCDEF
    //                                 xxxxxx|||xxxxxxxyyyyyy|||yyyyyyy
    //                                 0123456789ABCDEF0123456789ABCDEF
    //                                         xxxxxxxxxxxxxxxxyyyyyyyyyyyyyyyy
    EXPECT_EQ(received,               "00000000111111111111111100000000000000000");
    EXPECT_EQ(resync_in_samples, 0); // two bits were 1,0 (ack) - so resync at next sample (0)

    EXPECT_EQ(received_bits, "100"); // Ack is 1, 0, then there's padding to ensure that the right
    // output appears and this appears as an extra 0.
}

TEST_F(OversampledTxRxPinTest, PerfectInputAck) {
    //                                 0123456789ABCDEF0123456789ABCDEF
    //                                 xxxxxx|||xxxxxxxyyyyyy|||yyyyyyy
    const std::string input_samples = "11111111111111110000000000000000000000000";
    const std::string received = send_input_get_output(input_samples);
    expect_ack(received, m_o_pin._resync_in_samples(), m_received_bits);
}

TEST_F(OversampledTxRxPinTest, MajorityVote7Ack) { // Also all-bits majority test
    //                                 0123456789ABCDEF0123456789ABCDEF
    //                                 xxxxxx|||xxxxxxxyyyyyy|||yyyyyyy
    const std::string input_samples = "10000011100000000000000000000000000000000";
    const std::string received = send_input_get_output(input_samples);
    expect_ack(received, m_o_pin._resync_in_samples(), m_received_bits);
}

TEST_F(OversampledTxRxPinTest, MajorityVote3Ack) {
    //                                 0123456789ABCDEF0123456789ABCDEF
    //                                 xxxxxx|||xxxxxxxyyyyyy|||yyyyyyy
    const std::string input_samples = "10000001100000000000000000000000000000000";
    const std::string received = send_input_get_output(input_samples);
    expect_ack(received, m_o_pin._resync_in_samples(), m_received_bits);
}

TEST_F(OversampledTxRxPinTest, MajorityVote6Ack) {
    //                                 0123456789ABCDEF0123456789ABCDEF
    //                                 xxxxxx|||xxxxxxxyyyyyy|||yyyyyyy
    const std::string input_samples = "10000011000000000000000000000000000000000";
    const std::string received = send_input_get_output(input_samples);
    expect_ack(received, m_o_pin._resync_in_samples(), m_received_bits);
}

TEST_F(OversampledTxRxPinTest, MajorityVote5Ack) {
    //                                 0123456789ABCDEF0123456789ABCDEF
    //                                 xxxxxx|||xxxxxxxyyyyyy|||yyyyyyy
    const std::string input_samples = "10000010100000000000000000000000000000000";
    const std::string received = send_input_get_output(input_samples);
    expect_ack(received, m_o_pin._resync_in_samples(), m_received_bits);
}

// One One - This is the start of a data frame; want to ensure the resync is at the end of the data...

TEST_F(OversampledTxRxPinTest, SyncPulseInputOneOne) {
    //                                 0123456789ABCDEF0123456789ABCDEF
    //                                 xxxxxx|||xxxxxxxyyyyyy|||yyyyyyy
    const std::string input_samples = "1000001110000000000000111000000000000000";
    const std::string received = send_input_get_output(input_samples);

    //                                 0123456789ABCDEF0123456789ABCDEF
    //                                         xxxxxxxxxxxxxxxxyyyyyyyyyyyyyyyy
    EXPECT_EQ(received,               "0000000011111111111111111111111111111111");
    // data (full frame 11 bits [ 1 1 x x x x x x x x 0 ], so 16x11=176 samples)
    // but we've seen 40 bits, so take those off: 176-40=136.
    EXPECT_EQ(m_o_pin._resync_in_samples(), 136);
}

std::string stretch_16(const std::string &bits) {
    std::string samples;
    for (char bit : bits) {
        if (bit == '1') {
            samples.append("1111111111111111");
        } else {
            samples.append("0000000000000000");
        }
    }
    return samples;
}


TEST_F(OversampledTxRxPinTest, PerfectInputData) {
    // Two start bits, 8 bits of C9, a stop bit, and a trailer so we get all the data after the
    // majority vote 'delay'.
    const std::string delay_padding = "00000000";
    const std::string input_samples = stretch_16("11110010010") + delay_padding;
    const std::string received = send_input_get_output(input_samples);

    EXPECT_EQ(received, delay_padding + stretch_16("11110010010"));
    EXPECT_EQ(m_o_pin._resync_in_samples(), 0); // have been waiting for the next rising edge but then there was padding...
    EXPECT_EQ(m_received_bits, "11110010010");
}

TEST_F(OversampledTxRxPinTest, ResyncAtEndOfData_OneBitShort) {
    const std::string input_samples = stretch_16("1111001001"); // one bit short
    EXPECT_EQ(input_samples.size(), 160);

    send_input_get_output(input_samples);

    EXPECT_EQ(m_o_pin._resync_in_samples(), 16); // just about to start waiting for the next rising edge
}

TEST_F(OversampledTxRxPinTest, ResyncAtEndOfData_OneSampleShort) {
    std::string input_samples = stretch_16("11110010010");
    input_samples.pop_back(); // one sample short
    // data (full frame 11 bits [ 1 1 x x x x x x x x 0 ], so 16x11=176 samples)
    EXPECT_EQ(input_samples.size(), 175);

    send_input_get_output(input_samples);

    EXPECT_EQ(m_o_pin._resync_in_samples(), 1); // about to start waiting for the next rising edge after next sample
}

TEST_F(OversampledTxRxPinTest, ResyncAtEndOfData_FullDataFrame) {
    const std::string input_samples = stretch_16("11110010010");
    // data (full frame 11 bits [ 1 1 x x x x x x x x 0 ], so 16x11=176 samples)
    EXPECT_EQ(input_samples.size(), 176);

    send_input_get_output(input_samples);

    EXPECT_EQ(m_o_pin._resync_in_samples(), 0); // just started waiting for the next rising edge
}

TEST_F(OversampledTxRxPinTest, PerfectInputAckNoListener) {
    // Knock out the listener (the test setup always installs one)
    // Test that the listener is only called if it's not nullptr.
    // Without that check, this test crashes.
    m_o_pin.unregisterRxBitReceiver();
    //                                 0123456789ABCDEF0123456789ABCDEF
    //                                 xxxxxx|||xxxxxxxyyyyyy|||yyyyyyy
    const std::string input_samples = "11111111111111110000000000000000000000000";
    send_input_get_output(input_samples);
    EXPECT_EQ(m_received_bits, ""); // Tested as 100 when we listen.
}

// TODO better start bit detection in the OversampledTxRxPin - on receipt of the rising edge,
// only start detection if the majority vote of the start bit is correct. This will prevent noise
// at the starting sample triggering detection. (As mentioned in the Serial Communications book)

// TODO noisy cases of majority voting, all boundary cases

class DataAckSenderTest : public ::testing::Test {
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

    CrosswiredTxRxPinPair m_pair;
};

// TODO these tests will be rewritten in terms of the new design....

TEST_F(DataAckSenderTest, AckCanBeSent) {
    TxRxPin & pairA = m_pair.pairA();
    TxRxPin & pairB = m_pair.pairB();
    ClockingPinTracer trace(pairB);
    DataAckSender sender(pairA);
    EXPECT_EQ(sender.state(), DataAckSenderState::IDLE);

    sender.sendAck(); // Needs clocking to send the signals out. 16 clock pulses/bit.

    // An ack is a 1 followed by a 0.
    const int expected_bits = 2;
    const int expected_samples = expected_bits * 16;
    EXPECT_EQ(sender.state(), DataAckSenderState::SENDING);
    EXPECT_EQ(sender._queueLength(), 2);
    EXPECT_EQ(sender._data(), 0x0001);
    for (int i=0; i<expected_samples; i++) {
        EXPECT_EQ(sender.state(), DataAckSenderState::SENDING);

        sender.clock();
        trace.clock();
    }
    EXPECT_EQ(sender.state(), DataAckSenderState::IDLE);
    EXPECT_EQ(sender._queueLength(), 0);
    const std::vector<bool> heard = trace.heard();
    EXPECT_EQ(heard.size(), expected_samples);
    const int expected_ack_bits[expected_bits] = { 1, 0 };
    const std::vector<bool> expected = generate_sample_vector_from_bits(expected_ack_bits, expected_bits);
    EXPECT_EQ(heard, expected);
}

TEST_F(DataAckSenderTest, DataCanBeSent) {
    TxRxPin & pairA = m_pair.pairA();
    TxRxPin & pairB = m_pair.pairB();
    ClockingPinTracer trace(pairB);
    DataAckSender sender(pairA);
    EXPECT_EQ(sender.state(), DataAckSenderState::IDLE);

    sender.sendData(0xC9); // Needs clocking to send the signals out. 16 clock pulses/bit.

    // Data is two start bits (1), the data bits, then a stop bit (0).
    const int expected_bits = 11;
    const int expected_samples = expected_bits * 16;
    EXPECT_EQ(sender.state(), DataAckSenderState::SENDING);
    EXPECT_EQ(sender._queueLength(), expected_bits);
    EXPECT_EQ(sender._data(), 0x0327);
    logDebug("Data to be clocked out looks right with framing");
    for (int i=0; i<expected_samples; i++) {
        EXPECT_EQ(sender.state(), DataAckSenderState::SENDING);

        sender.clock();
        trace.clock();
    }
    EXPECT_EQ(sender.state(), DataAckSenderState::IDLE);
    EXPECT_EQ(sender._queueLength(), 0);
    const std::vector<bool> heard = trace.heard();
    EXPECT_EQ(heard.size(), expected_samples);   //       vvv note LSB first vvv
    const int expected_ack_bits[expected_bits] = { 1, 1,  1, 0, 0, 1, 0, 0, 1, 1,  0 };
    const std::vector<bool> expected = generate_sample_vector_from_bits(expected_ack_bits, expected_bits);
    EXPECT_EQ(heard, expected);
}

// TODO how to signal you can't ack or send data in a non-idle state



class DataAckReceiverTest : public ::testing::Test, AckReceiver, SendAckReceiver {
protected:

    void SetUp() override {
        setLogLevel(LOGLEVEL_DEBUG);
        logDebug("SetUp start");
        TxRxPin & pairA = m_pair.pairA();
        TxRxPin & pairB = m_pair.pairB();
        m_receiver = new DataAckReceiver(pairA);
        m_receiver->registerAckReceiver(*this); // dereference this to get a reference
        m_receiver->registerSendAckReceiver(*this);
        logDebug("Setup complete");
        logFlush();
    }

    void TearDown() override {
        logDebug("TearDown start");
        logDebug("TearDown complete");
        logFlush();
    }

    // AckReceiver
    void ackReceived() override {
        logDebug("Ack received");
        m_acks_received++;
    }

    // SendAckReceiver
    bool requestSendAck() override {
        logDebugF("Send Ack requested - returning %d", m_request_send_ack_response);
        m_send_acks_received++;
        return m_request_send_ack_response;
    }

    void goToStartBit2() {
        // Enter START_BIT_2
        m_receiver->bitStateReceived(true);
    }

    void goToData() {
        goToStartBit2();
        // Enter DATA
        setRequestSendAckResponse(true); // there is space to receive
        m_receiver->bitStateReceived(true);
        EXPECT_EQ(m_receiver->state(), DataAckReceiverState::DATA);
    }

    void setRequestSendAckResponse(const bool response) {
        m_request_send_ack_response = response;
    }

    CrosswiredTxRxPinPair m_pair;
    DataAckReceiver *m_receiver = nullptr;
    int m_acks_received = 0;
    int m_send_acks_received = 0;
    bool m_request_send_ack_response = true;
};

TEST_F(DataAckReceiverTest, InitialConditions) {
    EXPECT_EQ(m_receiver->state(), DataAckReceiverState::IDLE);
    EXPECT_EQ(m_receiver->_bit_count(), -1);
    EXPECT_EQ(m_receiver->_buffer(), 0x69);

    EXPECT_EQ(m_acks_received, 0);
    EXPECT_EQ(m_send_acks_received, 0);
}

TEST_F(DataAckReceiverTest, IdleReceivesHighGoesToStartBit2) {
    m_receiver->bitStateReceived(true);
    EXPECT_EQ(m_receiver->state(), DataAckReceiverState::START_BIT_2);
}

TEST_F(DataAckReceiverTest, StartBit2ReceivesLowAcksGoesToIdle) {
    // Enter START_BIT_2
    m_receiver->bitStateReceived(true);

    // Back to IDLE - that should have generated an ack; our callback
    // would have been called, incrementing the count.
    m_receiver->bitStateReceived(false);

    EXPECT_EQ(m_receiver->state(), DataAckReceiverState::IDLE);
    EXPECT_EQ(m_acks_received, 1);
}

TEST_F(DataAckReceiverTest, StartBit2ReceivesLowNoAckReceiverGoesToIdle) {
    // Knock out the listener (the test setup always installs one)
    // Test that the listener is only called if it's not nullptr.
    // Without that check, this test crashes.
    m_receiver->unregisterAckReceiver();
    goToStartBit2();

    // Back to IDLE - that should have generated an ack; our callback
    // would have been called, if we had one.
    m_receiver->bitStateReceived(false);

    EXPECT_EQ(m_receiver->state(), DataAckReceiverState::IDLE);
    EXPECT_EQ(m_acks_received, 0); // no ack listener
}

TEST_F(DataAckReceiverTest, StartBit2ReceivesHighCallsSendAckGoesToData) {
    goToStartBit2();

    // The receiver will call the internal 'send ack' callback, notifying
    // the DataAckSender to send an ack as we can receive this data. This
    // will return true (the test fixture default), so there is enough
    // space to receive.
    m_receiver->bitStateReceived(true);

    EXPECT_EQ(m_receiver->state(), DataAckReceiverState::DATA);
    EXPECT_EQ(m_send_acks_received, 1);
    EXPECT_EQ(m_receiver->_bit_count(), 0);
    EXPECT_EQ(m_receiver->_buffer(), 0x00);
}

TEST_F(DataAckReceiverTest, StartBit2ReceivesHighNoSendAckReceiverGoesToDiscard) {
    // Knock out the listener (the test setup always installs one)
    // Test that the listener is only called if it's not nullptr.
    // Without that check, this test crashes.
    m_receiver->unregisterSendAckReceiver();
    goToStartBit2();

    // The receiver would call the internal 'send ack' callback, notifying
    // the DataAckSender to send an ack as we can receive this data. But we
    // don't have one, so have to assume there's no space to receive this
    // data, and no ability to ack. So, reject the data by going into
    // DISCARD.
    m_receiver->bitStateReceived(true);

    EXPECT_EQ(m_receiver->state(), DataAckReceiverState::DISCARD);
    EXPECT_EQ(m_send_acks_received, 0);
    EXPECT_EQ(m_receiver->_bit_count(), 0);
    EXPECT_EQ(m_receiver->_buffer(), 0x00);
}

TEST_F(DataAckReceiverTest, StartBit2ReceivesHighCallsSendAckGoesToDiscard) {
    goToStartBit2();

    // The receiver will call the internal 'send ack' callback, requesting
    // the DataAckSender to send an ack - but this
    // will return false to indicate no available space to receive.
    setRequestSendAckResponse(false);
    m_receiver->bitStateReceived(true);

    EXPECT_EQ(m_receiver->state(), DataAckReceiverState::DISCARD);
    EXPECT_EQ(m_send_acks_received, 1); // The callback was called.
    EXPECT_EQ(m_receiver->_bit_count(), 0); // Discard needs this zeroing.
    EXPECT_EQ(m_receiver->_buffer(), 0x00);
}

TEST_F(DataAckReceiverTest, DataToStopBit) {
    goToData();

    m_receiver->bitStateReceived(true);
    EXPECT_EQ(m_receiver->_bit_count(), 1);
    EXPECT_EQ(m_receiver->_buffer(), 0b00000001);

    m_receiver->bitStateReceived(true);
    EXPECT_EQ(m_receiver->_bit_count(), 2);
    EXPECT_EQ(m_receiver->_buffer(), 0b00000011);

    m_receiver->bitStateReceived(false);
    EXPECT_EQ(m_receiver->_bit_count(), 3);
    EXPECT_EQ(m_receiver->_buffer(), 0b00000110);

    m_receiver->bitStateReceived(false);
    EXPECT_EQ(m_receiver->_bit_count(), 4);
    EXPECT_EQ(m_receiver->_buffer(), 0b00001100);

    m_receiver->bitStateReceived(false);
    EXPECT_EQ(m_receiver->_bit_count(), 5);
    EXPECT_EQ(m_receiver->_buffer(), 0b00011000);

    m_receiver->bitStateReceived(false);
    EXPECT_EQ(m_receiver->_bit_count(), 6);
    EXPECT_EQ(m_receiver->_buffer(), 0b00110000);

    m_receiver->bitStateReceived(true);
    EXPECT_EQ(m_receiver->_bit_count(), 7);
    EXPECT_EQ(m_receiver->_buffer(), 0b01100001);
    EXPECT_EQ(m_receiver->state(), DataAckReceiverState::DATA);

    m_receiver->bitStateReceived(true);
    EXPECT_EQ(m_receiver->_bit_count(), 8);
    EXPECT_EQ(m_receiver->_buffer(), 0b11000011);
    EXPECT_EQ(m_receiver->state(), DataAckReceiverState::STOP_BIT);
}
