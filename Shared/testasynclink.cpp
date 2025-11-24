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
#include <atomic>
#include <chrono>
#include <thread>

#include "gtest/gtest.h"
#include "link.h"
#include "asynclink.h"
#include "log.h"
#include "misc.h"

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


class DisconnectedPin: public TxRxPin {
public:
    DisconnectedPin() {

    }

    ~DisconnectedPin() override = default;

    bool getRx() override {
        return false;
    }

    void setTx(const bool state) override {
    }
};


class AsyncLinkClockTest : public ::testing::Test {
public:
    AsyncLinkClockTest() : clock(-1, handler) {
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

    class AtomicTickHandler: public TickHandler {
    public:
        AtomicTickHandler() : TickHandler(), m_counter(0) {

        }

        // TickHandler
        void tick() override {
            // logDebug("Tick - increment");
            m_counter.fetch_add(1);
        }

        int counter() {
            int counter = m_counter.load();
            logDebugF("Counter is %d", counter);
            return counter;
        }

    private:
        std::atomic_int m_counter;
    };

    class AtomicClockable: public Link {
    public:
        AtomicClockable(int linkNo, bool isServer) : Link(linkNo, isServer), m_counter(0) {};
        void initialise() override {};
        ~AtomicClockable() override = default;
        BYTE8 readByte() override { return 0; };
        void writeByte(BYTE8 b) override {};
        void resetLink() override {};
        int getLinkType() override { return 0; };

        void clock() override {
            // logDebug("Clock - increment");
            m_counter.fetch_add(1);
        }

        int counter() {
            int counter = m_counter.load();
            logDebugF("Counter is %d", counter);
            return counter;
        }
    private:
        std::atomic_int m_counter;
    };

    AtomicTickHandler handler;
    AsyncLinkClock clock;
};

TEST_F(AsyncLinkClockTest, InitialConditions) {
    EXPECT_EQ(handler.counter(), 0);
    EXPECT_EQ(handler.is_running(), false);
}

TEST_F(AsyncLinkClockTest, StartTicksThenStopHalts) {
    clock.start();
    EXPECT_EQ(handler.is_running(), true);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_EQ(handler.is_running(), true);
    int counter1 = handler.counter();
    EXPECT_GT(counter1, 0); // counter going up

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    int counter2 = handler.counter();
    EXPECT_GT(counter2, counter1); // counter still going up

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    clock.stop();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(handler.is_running(), false); // now everything is stopping
    int counter3 = handler.counter();
    EXPECT_GE(counter3, counter2); // maybe greater

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(handler.is_running(), false); // should be stasis.
    int counter4 = handler.counter();
    EXPECT_EQ(counter4, counter3); // no change
}

TEST_F(AsyncLinkClockTest, MultipleTickHandler) {
    MultipleTickHandler mth;
    AtomicClockable clockable(0, false);
    EXPECT_EQ(clockable.counter(), 0);
    mth.tick();
    EXPECT_EQ(clockable.counter(), 0);
    mth.addLink(&clockable);
    mth.tick();
    EXPECT_EQ(clockable.counter(), 1);

    mth.addLink(&clockable); // it'll be called twice
    mth.tick();
    EXPECT_EQ(clockable.counter(), 3);
}

class AsyncLinkTest : public ::testing::Test {
public:
    AsyncLinkTest() : clock(-1, handler) {

    }
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
        linkB = new AsyncLink(1, false, pairB);
        linkB->setDebug(true);
        logDebug("Initialising Link B");
        linkB->initialise();

        logDebug("Setup and start clock");
        handler.addLink(&*linkA);
        handler.addLink(&*linkB);
        clock.start();

        logDebug("Setup complete");
        logFlush();
    }

    void TearDown() override {
        logDebug("TearDown start; stopping clock");
        clock.stop();

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

    void pause() {
        std::this_thread::sleep_for(std::chrono::microseconds(LINK_CLOCK_TICK_INTERVAL_US));
    }

    CrosswiredTxRxPinPair pair;
    AsyncLink *linkA = nullptr;
    AsyncLink *linkB = nullptr;
    MultipleTickHandler handler;
    AsyncLinkClock clock;
};

TEST_F(AsyncLinkTest, RTSSetOnInitialisation) {
    EXPECT_EQ(linkA->queryReadyToSend(), true);
}

TEST_F(AsyncLinkTest, ClearRTSClearsIt) {
    linkA->clearReadyToSend();
    EXPECT_EQ(linkA->queryReadyToSend(), false);
}

TEST_F(AsyncLinkTest, SetRTSSetsIt) {
    linkA->clearReadyToSend();
    linkA->setReadyToSend();
    EXPECT_EQ(linkA->queryReadyToSend(), true);
}

TEST_F(AsyncLinkTest, RTSClearedWhenDataSentAsync) {
    bool ok = linkA->writeByteAsync(0xC9,
[](bool ack, bool to) {
        // Handle callback
    });
    EXPECT_EQ(ok, true);
    pause();
    EXPECT_EQ(linkA->queryReadyToSend(), false);
}

TEST_F(AsyncLinkTest, DataSentAsyncGetsAckedRTSSet) {
    linkA->writeByteAsync(0xC9,
    [](bool ack, bool to) {
        // Handle callback
    });
    for (int i=0; i<24 * 12; i++) { // 24 bit-lengths should be enough to hear the ack
        pause();
    }
    EXPECT_EQ(linkA->queryReadyToSend(), true);
}

TEST_F(AsyncLinkTest, DataSentAsyncGetsAckedAndCalledBack) {
    bool acked = false;
    linkA->writeByteAsync(0xC9,
    [&](bool ack, bool to) {
        // Handle callback
        acked = ack;
    });
    for (int i=0; i<24 * 12; i++) { // 24 bit-lengths should be enough to hear the ack
        pause();
    }
    EXPECT_EQ(acked, true);
}

TEST_F(AsyncLinkTest, DataSentAsyncGetsAckedAndCalledBackNullSafety) {
    linkA->writeByteAsync(0xC9, nullptr);
    for (int i=0; i<24 * 12; i++) { // 24 bit-lengths should be enough to hear the ack
        pause();
    }
    // Got here? Good!
}

TEST_F(AsyncLinkTest, StartWritingAsync) {
    linkA->writeByteAsync(0xC9,
        [](bool ack, bool to) {
        // Handle callback
    });

    pause();
    pause();
    pause();

    TxRxPin &pairB = pair.pairB();
    EXPECT_EQ(pairB.getRx(), true);
}

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

class AsyncLinkTimeoutTest : public ::testing::Test {
public:
    AsyncLinkTimeoutTest() : clock(-1, handler) {

    }
protected:
    void SetUp() override {
        setLogLevel(LOGLEVEL_DEBUG);
        logDebug("SetUp start");

        linkA = new AsyncLink(0, false, pin);
        linkA->setDebug(true);
        logDebug("Initialising Link");
        linkA->initialise();

        logDebug("Setup and start clock");
        handler.addLink(&*linkA);
        clock.start();

        logDebug("Setup complete");
        logFlush();
    }

    void TearDown() override {
        logDebug("TearDown start; stopping clock");
        clock.stop();

        if (linkA != nullptr) {
            logDebug("Resetting Link");
            linkA->resetLink();
            delete linkA;
        }
        logDebug("TearDown complete");
        logFlush();
    }

    void pause() {
        std::this_thread::sleep_for(std::chrono::microseconds(LINK_CLOCK_TICK_INTERVAL_US));
    }
    DisconnectedPin pin;
    AsyncLink *linkA = nullptr;
    MultipleTickHandler handler;
    AsyncLinkClock clock;
};

TEST_F(AsyncLinkTimeoutTest, DisconnectedOps) {
    DisconnectedPin pin;
    EXPECT_EQ(pin.getRx(), false);
    pin.setTx(true);
}

TEST_F(AsyncLinkTimeoutTest, DataSentAsyncTimesOutWithNoOtherLinkListening) {
    bool timeout = false;
    linkA->writeByteAsync(0xC9, [&](bool ack, bool to) {
        // Handle callback
        timeout = to;
    });
    for (int i=0; i<24 * 12; i++) { // 24 bit-lengths should be enough to time out
        pause();
    }
    linkA->writeByteAsync(0xC9, nullptr);
    EXPECT_EQ(timeout, true);
}

TEST_F(AsyncLinkTimeoutTest, DataSentAsyncTimesOutWithNoOtherLinkListeningNullSafety) {
    linkA->writeByteAsync(0xC9, nullptr);
    for (int i=0; i<24 * 12; i++) { // 24 bit-lengths should be enough to time out
        pause();
    }
    // Just gets through the night without crashing
}



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
// Zero, Zero - invalid
// Zero,  One - invalid
// One,  Zero - ack
// One,   One - data

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

// One, One - This is the start of a data frame; want to ensure the resync is at the end of the data...

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

// Single-letter comments relate the test to the labelled transitions on the statechart.
// Lower case letters are not implemented / covered by tests yet.
// ABCDEFGHIjKlMNoPQRstuv
class DataAckSenderTest : public ::testing::Test, SenderToLink {
protected:

    void SetUp() override {
        setLogLevel(LOGLEVEL_DEBUG);
        logDebug("SetUp start");
        TxRxPin & pairA = m_pair.pairA();
        TxRxPin & pairB = m_pair.pairB();
        m_trace = new ClockingPinTracer(pairB);
        m_sender = new DataAckSender(0, pairA);
        m_sender->registerSenderToLink(*this); // dereference this to get a reference

        logDebug("Setup complete");
        logFlush();
    }

    void TearDown() override {
        logDebug("TearDown start");
        logDebug("TearDown complete");
        logFlush();
    }

    // SenderToLink
    bool queryReadyToSend() override {
        return m_ready_to_send;
    }

    void setReadyToSend() override {
        m_ready_to_send = true;
    }

    void clearReadyToSend() override {
        m_ready_to_send = false;
    }

    void setTimeoutError() override {
        m_timeout = true;
    }

    void goToAckTimeout() const {
        m_sender->sendData(0xC9);
        constexpr int expected_bits = 11;
        constexpr int expected_samples = expected_bits * 16;
        for (int i=0; i<expected_samples; i++) {
            m_sender->clock();
            m_trace->clock();
        }
        EXPECT_EQ(m_sender->state(), DataAckSenderState::ACK_TIMEOUT);
    }

    CrosswiredTxRxPinPair m_pair;
    ClockingPinTracer *m_trace = nullptr;
    DataAckSender *m_sender = nullptr;
    bool m_ready_to_send = false;
    bool m_timeout = false;
};

TEST_F(DataAckSenderTest, InitialConditions) {
    EXPECT_EQ(m_sender->state(), DataAckSenderState::IDLE);
    EXPECT_EQ(m_sender->_ack_rxed(), false);
    EXPECT_EQ(m_sender->_send_ack(), false);
    EXPECT_EQ(m_sender->_data_enqueued(), false);
    // Instantiating (registering) sets RTS
    EXPECT_EQ(m_ready_to_send, true);
}

// A
TEST_F(DataAckSenderTest, DataSentAckReceivedGoesToIdle) {
    EXPECT_EQ(m_sender->sendData(0xC9), true);
    constexpr int expected_bits = 11;
    constexpr int expected_samples = expected_bits * 16;
    logDebug("Data to be clocked out looks right with framing");
    for (int i=0; i<expected_samples - 1; i++) { // -1 to sense ack_rxed flag before going IDLE and resetting
        m_sender->clock();
        m_trace->clock();
        // Ack would be signalled after the receiver has detected the second start bit,
        // so after 16 (first bit) + 9 (end of majority detection of second bit) = 25.

        if (i == 25) {
            logDebug("Pretend an ack has been seen");
            m_sender->ackReceived();
        }
    }
    // Just before going idle, has the sender seen that ack?
    EXPECT_EQ(m_sender->_ack_rxed(), true);
    // Test rest of transition guard
    EXPECT_EQ(m_sender->_send_ack(), false);
    // A bit more clocking...
    m_sender->clock();
    m_trace->clock();

    EXPECT_EQ(m_sender->state(), DataAckSenderState::IDLE);
    EXPECT_EQ(m_sender->_ack_rxed(), false);
}

// B
TEST_F(DataAckSenderTest, DataSentInIdleGoesToSendingData) {
    // RTS sensed true as part of initial conditions.
    EXPECT_EQ(m_sender->sendData(0xC9), true); // Needs clocking to send the signals out. 16 clock pulses/bit.

    EXPECT_EQ(queryReadyToSend(), false); // We're sending now, so disallow further until it's acked.
    EXPECT_EQ(m_sender->_ack_rxed(), false);
    EXPECT_EQ(m_sender->state(), DataAckSenderState::SENDING_DATA);
}

// C
TEST_F(DataAckSenderTest, SendAckReceivedInIdleGoesToSendingAck) {
    m_sender->sendAck();

    EXPECT_EQ(m_sender->state(), DataAckSenderState::SENDING_ACK);
    EXPECT_EQ(m_sender->_queueLength(), 2);
    EXPECT_EQ(m_sender->_data(), 0x0001);
}

// D
TEST_F(DataAckSenderTest, DataReceivedInIdleWithFalseRTSStaysIdle) {
    clearReadyToSend();
    EXPECT_EQ(m_sender->sendData(0xc9), false);

    EXPECT_EQ(m_sender->state(), DataAckSenderState::IDLE);
}

// D (null safety)
TEST_F(DataAckSenderTest, DataSendButNoListener) {
    m_sender->unregisterSenderToLink();
    EXPECT_EQ(m_sender->sendData(0xC9), false);
}

// E
TEST_F(DataAckSenderTest, NoDataEnqueuedSendingAckClocksAckOutGoesIdle) {
    // data_enqueued is false in initial conditions
    m_sender->sendAck();
    constexpr int expected_bits = 2;
    constexpr int expected_samples = expected_bits * 16;
    for (int i=0; i<expected_samples; i++) {
        EXPECT_EQ(m_sender->state(), DataAckSenderState::SENDING_ACK);
        m_sender->clock();
        m_trace->clock();
    }

    // There's no data_enqueued so back to idle.
    EXPECT_EQ(m_sender->state(), DataAckSenderState::IDLE);
}

// F
TEST_F(DataAckSenderTest, SendingAckDueToSendAckAndAckReceivedGoesToIdle) {
    // Want to get into SENDING_ACK with send_ack true.
    m_sender->sendData(0xC9);
    constexpr int expected_data_bits = 11;
    constexpr int expected_data_samples = expected_data_bits * 16;
    for (int i=0; i<expected_data_samples - 1; i++) { // -1 so we stay in SENDING_DATA while send ack cb called.
        m_sender->clock();
        m_trace->clock();
    }
    EXPECT_EQ(m_sender->state(), DataAckSenderState::SENDING_DATA);
    EXPECT_EQ(m_sender->_send_ack(), false);
    m_sender->sendAck();
    EXPECT_EQ(m_sender->_send_ack(), true);
    // One more clock to get out of SENDING_DATA
    m_sender->clock();
    m_trace->clock();
    EXPECT_EQ(m_sender->state(), DataAckSenderState::SENDING_ACK);

    constexpr int expected_ack_bits = 2;
    constexpr int expected_ack_samples = expected_ack_bits * 16;
    for (int i=0; i<expected_ack_samples - 1; i++) { // -1 to stay in SENDING_ACK while we receive an ack.
        m_sender->clock();
        m_trace->clock();
    }
    // Have we received an ack yet?
    EXPECT_EQ(m_sender->_ack_rxed(), false);
    // Here it is!
    m_sender->ackReceived();
    EXPECT_EQ(m_sender->_ack_rxed(), true);
    // One more clock to get out of SENDING_ACK
    m_sender->clock();
    m_trace->clock();

    // There's no data_enqueued, and we came into SENDING_ACK via send_ack, and there's an ack received, so back to idle.
    EXPECT_EQ(m_sender->state(), DataAckSenderState::IDLE);
    // Leaving SENDING_ACK clears send_ack
    EXPECT_EQ(m_sender->_send_ack(), false);
}

// G
TEST_F(DataAckSenderTest, DataEnqueuedWhenSendingAckClocksAckOutThenGoesSendingData) {
    m_sender->sendAck();
    EXPECT_EQ(m_sender->_data_enqueued(), false);
    EXPECT_EQ(m_sender->state(), DataAckSenderState::SENDING_ACK);
    // Enqueue some data while we're sending the ack...
    EXPECT_EQ(m_sender->sendData(0xC9), true);
    EXPECT_EQ(m_sender->_data_enqueued(), true);
    EXPECT_EQ(m_sender->_data_enqueued_buffer(), 0xC9);

    // Needs clocking to send the signals out. 16 clock pulses/bit.
    // An ack is a 1 followed by a 0.
    constexpr int expected_bits = 2;
    constexpr int expected_samples = expected_bits * 16;
    for (int i=0; i<expected_samples; i++) {
        EXPECT_EQ(m_sender->state(), DataAckSenderState::SENDING_ACK);
        m_sender->clock();
        m_trace->clock();
    }
    const std::vector<bool> heard = m_trace->heard();
    EXPECT_EQ(heard.size(), expected_samples);
    constexpr int expected_ack_bits[expected_bits] = { 1, 0 };
    const std::vector<bool> expected = generate_sample_vector_from_bits(expected_ack_bits, expected_bits);
    EXPECT_EQ(heard, expected);

    // There's data_enqueued so we should be ready to send it.
    EXPECT_EQ(m_sender->state(), DataAckSenderState::SENDING_DATA);
    EXPECT_EQ(m_sender->_queueLength(), 11);
    EXPECT_EQ(m_sender->_data(), 0x0327); // Data is two start bits (1), the data bits, then a stop bit (0).
    EXPECT_EQ(m_sender->_ack_rxed(), false);
    EXPECT_EQ(m_sender->_data_enqueued(), false); // These get reset after we've transitioned.
    EXPECT_EQ(m_sender->_data_enqueued_buffer(), 0x00);
}

// H
TEST_F(DataAckSenderTest, DataSendThenSendAck) {
    EXPECT_EQ(m_sender->sendData(0xC9), true);
    constexpr int num_expected_data_bits = 11;
    constexpr int num_expected_data_samples = num_expected_data_bits * 16;
    for (int i=0; i<num_expected_data_samples - 1; i++) { // -1 to sense send_ack flag before leaving SENDING_DATA
        m_sender->clock();
        m_trace->clock();
        // If the receiver is receiving data, and asks the sender to send an ack.
        // Ack would be requested after the receiver has detected the second start bit,
        // so after 16 (first bit) + 9 (end of majority detection of second bit) = 25.
        if (i == 25) {
            logDebug("Please ack imaginary received data");
            m_sender->sendAck();
        }
    }
    EXPECT_EQ(m_sender->state(), DataAckSenderState::SENDING_DATA);
    // Has the sender registered it needs to send the ack?
    EXPECT_EQ(m_sender->_send_ack(), true);
    // A bit more clocking...
    m_sender->clock();
    m_trace->clock();

    logInfoF("the state is now %s", DataAckSenderStateToString(m_sender->state()));
    EXPECT_EQ(m_sender->state(), DataAckSenderState::SENDING_ACK);
    // Still recall you were told to send the ack?
    EXPECT_EQ(m_sender->_send_ack(), true);
    constexpr int num_expected_ack_bits = 2;
    EXPECT_EQ(m_sender->_queueLength(), num_expected_ack_bits);
    EXPECT_EQ(m_sender->_data(), 0x01);
}

// I
TEST_F(DataAckSenderTest, DataSentButNoAckReceived) {
    EXPECT_EQ(m_sender->sendData(0xC9), true);
    EXPECT_EQ(m_sender->state(), DataAckSenderState::SENDING_DATA);
    EXPECT_EQ(m_sender->_ack_rxed(), false);
    EXPECT_EQ(m_sender->_send_ack(), false);
    constexpr int expected_bits = 11;
    constexpr int expected_samples = expected_bits * 16;
    for (int i=0; i<expected_samples; i++) {
        m_sender->clock();
        m_trace->clock();
    }
    EXPECT_EQ(m_sender->state(), DataAckSenderState::ACK_TIMEOUT);
    EXPECT_EQ(m_sender->_ack_rxed(), false);
}

// J
TEST_F(DataAckSenderTest, SendingAckToTimeout) {
    // Want to get into SENDING_ACK with send_ack true.
    EXPECT_EQ(m_sender->sendData(0xC9), true); // Needs clocking to send the signals out. 16 clock pulses/bit.
    constexpr int expected_bits = 11;
    constexpr int expected_samples = expected_bits * 16;
    for (int i=0; i<expected_samples - 1; i++) { // -1 so we stay in SENDING_DATA while send ack cb called.
        m_sender->clock();
        m_trace->clock();
    }
    m_sender->sendAck();
    EXPECT_EQ(m_sender->_send_ack(), true);
    EXPECT_EQ(m_sender->_ack_rxed(), false);
     // One more clock to get out of SENDING_DATA
    m_sender->clock();
    m_trace->clock();
    EXPECT_EQ(m_sender->state(), DataAckSenderState::SENDING_ACK);

    // Now sense the ack we're sending...
    constexpr int expected_ack_bits = 2;
    constexpr int expected_ack_samples = expected_ack_bits * 16;
    for (int i=0; i<expected_ack_samples; i++) {
        EXPECT_EQ(m_sender->state(), DataAckSenderState::SENDING_ACK);
        m_sender->clock();
        m_trace->clock();
    }

    // We never heard an ack to our SENDING_DATA.
    EXPECT_EQ(m_sender->state(), DataAckSenderState::ACK_TIMEOUT);
    // send_ack cleared on exit from SENDING_ACK
    EXPECT_EQ(m_sender->_send_ack(), false);
}

// K
TEST_F(DataAckSenderTest, EnterIdleWithAckRxedClearsItAndSetsRTS) {
    EXPECT_EQ(m_sender->sendData(0xC9), true);
    constexpr int expected_bits = 11;
    constexpr int expected_samples = expected_bits * 16;
    EXPECT_EQ(m_sender->state(), DataAckSenderState::SENDING_DATA);
    EXPECT_EQ(m_sender->_ack_rxed(), false);
    for (int i=0; i<expected_samples - 1; i++) { // -1 to sense ack_rxed flag before going IDLE and resetting
        m_sender->clock();
        m_trace->clock();

        // Ack would be signalled after the receiver has detected the second start bit,
        // so after 16 (first bit) + 9 (end of majority detection of second bit) = 25.
        if (i == 25) {
            logDebug("Pretend an ack has been seen");
            m_sender->ackReceived();
        }
    }
    // Just before going idle, has the sender seen that ack?
    EXPECT_EQ(m_sender->_ack_rxed(), true);
    // send_ack is not set, else we'd go to SENDING_ACK
    EXPECT_EQ(m_sender->_send_ack(), false);
    // RTS should be clear (we're sending)
    EXPECT_EQ(m_ready_to_send, false);
    // A bit more clocking...
    m_sender->clock();
    m_trace->clock();

    EXPECT_EQ(m_sender->state(), DataAckSenderState::IDLE);
    EXPECT_EQ(m_sender->_queueLength(), 0);
    EXPECT_EQ(m_sender->_ack_rxed(), false);
    // And the ready-to-send flag is set, allowing further sends.
    EXPECT_EQ(m_ready_to_send, true);
}

// L
TEST_F(DataAckSenderTest, AckInIdleJustLogged) {
    m_sender->ackReceived();
    // can't test this ;) manually verified
}

// M
TEST_F(DataAckSenderTest, DataEnqueuedWhenSendingAck) {
    m_sender->sendAck();
    EXPECT_EQ(m_sender->_data_enqueued(), false);
    EXPECT_EQ(m_sender->state(), DataAckSenderState::SENDING_ACK);
    // Throw in an ack to see ack_rxed getting cleared.
    m_sender->ackReceived();
    EXPECT_EQ(m_sender->_ack_rxed(), true);
    // Enqueue some data while we're sending the ack...
    EXPECT_EQ(m_sender->sendData(0xC9), true);
    EXPECT_EQ(m_sender->_data_enqueued(), true);
    EXPECT_EQ(m_sender->_data_enqueued_buffer(), 0xC9);
    // ack_rxed gets cleared down
    EXPECT_EQ(m_sender->_ack_rxed(), false);
}

// N
TEST_F(DataAckSenderTest, AckClockedOutInSendingAck) {
    m_sender->sendAck();
    EXPECT_EQ(m_sender->_data_enqueued(), false);
    EXPECT_EQ(m_sender->state(), DataAckSenderState::SENDING_ACK);

    // Needs clocking to send the signals out. 16 clock pulses/bit.
    // An ack is a 1 followed by a 0.
    constexpr int expected_bits = 2;
    constexpr int expected_samples = expected_bits * 16;
    constexpr int num_expected_ack_bits = 2;
    EXPECT_EQ(m_sender->_queueLength(), num_expected_ack_bits);
    EXPECT_EQ(m_sender->_data(), 0x01);
    for (int i=0; i<expected_samples; i++) {
        EXPECT_EQ(m_sender->state(), DataAckSenderState::SENDING_ACK);
        m_sender->clock();
        m_trace->clock();
    }
    const std::vector<bool> heard = m_trace->heard();
    EXPECT_EQ(heard.size(), expected_samples);
    constexpr int expected_ack_bits[expected_bits] = { 1, 0 };
    const std::vector<bool> expected = generate_sample_vector_from_bits(expected_ack_bits, expected_bits);
    EXPECT_EQ(heard, expected);
}

// O
TEST_F(DataAckSenderTest, AckReceivedInSendingAckSetsAckRxed) {
    m_sender->sendAck();
    EXPECT_EQ(m_sender->_data_enqueued(), false);
    EXPECT_EQ(m_sender->state(), DataAckSenderState::SENDING_ACK);
    EXPECT_EQ(m_sender->_ack_rxed(), false);

    m_sender->ackReceived();
    EXPECT_EQ(m_sender->_ack_rxed(), true);
}

// P
TEST_F(DataAckSenderTest, DataCantBeSentWhileAlreadySending) {
    EXPECT_EQ(m_sender->sendData(0xC9), true);
    constexpr int expected_bits = 11;
    constexpr int expected_samples = expected_bits * 16 - 1; // -1 so we're still in SENDING_DATA
    for (int i=0; i<expected_samples; i++) {
        m_sender->clock();
        m_trace->clock();
    }
    EXPECT_EQ(m_sender->state(), DataAckSenderState::SENDING_DATA);

    EXPECT_EQ(m_sender->sendData(0xAA), false); // Can't send if busy
}

// Q
TEST_F(DataAckSenderTest, DataClockedOutInSendingData) {
    EXPECT_EQ(m_sender->sendData(0xC9), true); // Needs clocking to send the signals out. 16 clock pulses/bit.

    // Data is two start bits (1), the data bits, then a stop bit (0).
    constexpr int expected_bits = 11;
    constexpr int expected_samples = expected_bits * 16;
    EXPECT_EQ(m_sender->state(), DataAckSenderState::SENDING_DATA);
    EXPECT_EQ(m_sender->_queueLength(), expected_bits);
    EXPECT_EQ(m_sender->_data(), 0x0327);
    EXPECT_EQ(m_sender->_ack_rxed(), false);
    for (int i=0; i<expected_samples; i++) {
        EXPECT_EQ(m_sender->state(), DataAckSenderState::SENDING_DATA);
        m_sender->clock();
        m_trace->clock();
    }

    const std::vector<bool> heard = m_trace->heard();
    EXPECT_EQ(heard.size(), expected_samples);   //       vvv note LSB first vvv
    const int expected_data_bits[expected_bits] = { 1, 1,  1, 0, 0, 1, 0, 0, 1, 1,  0 };
    const std::vector<bool> expected = generate_sample_vector_from_bits(expected_data_bits, expected_bits);
    EXPECT_EQ(heard, expected);
}

// R
TEST_F(DataAckSenderTest, AckReceivedInSendingData) {
    EXPECT_EQ(m_sender->sendData(0xC9), true);
    constexpr int expected_bits = 11;
    constexpr int expected_samples = expected_bits * 16;
    EXPECT_EQ(m_sender->_ack_rxed(), false);
    for (int i=0; i<expected_samples - 1; i++) { // -1 so we can receive an ack before leaving state
        EXPECT_EQ(m_sender->state(), DataAckSenderState::SENDING_DATA);
        m_sender->clock();
        m_trace->clock();
    }
    m_sender->ackReceived();
    EXPECT_EQ(m_sender->_ack_rxed(), true);
    // A bit more clocking
    m_sender->clock();
    m_trace->clock();
}

// S
TEST_F(DataAckSenderTest, SendAckRequestedInSendingData) {
    EXPECT_EQ(m_sender->sendData(0xC9), true);
    constexpr int expected_bits = 11;
    constexpr int expected_samples = expected_bits * 16;
    EXPECT_EQ(m_sender->_send_ack(), false);
    for (int i=0; i<expected_samples - 1; i++) { // -1 so we can have a send_ack request before leaving state
        EXPECT_EQ(m_sender->state(), DataAckSenderState::SENDING_DATA);
        m_sender->clock();
        m_trace->clock();
    }
    m_sender->sendAck();
    EXPECT_EQ(m_sender->_send_ack(), true);
    // A bit more clocking
    m_sender->clock();
    m_trace->clock();
}

// T
TEST_F(DataAckSenderTest, TimeoutEntrySetsTimeoutFlag) {
    EXPECT_EQ(m_timeout, false);
    EXPECT_EQ(m_sender->_ack_rxed(), false);
    goToAckTimeout();
    EXPECT_EQ(m_timeout, true);
}

// T (null safety)
TEST_F(DataAckSenderTest, TimeoutEntrySetsTimeoutFlagNoListener) {
    EXPECT_EQ(m_timeout, false);
    EXPECT_EQ(m_sender->_ack_rxed(), false);
    m_sender->sendData(0xC9);
    constexpr int expected_bits = 11;
    constexpr int expected_samples = expected_bits * 16;
    for (int i=0; i<expected_samples - 1; i++) { // -1 so we knock out the listener before state change
        EXPECT_EQ(m_sender->state(), DataAckSenderState::SENDING_DATA);
        m_sender->clock();
        m_trace->clock();
    }
    m_sender->unregisterSenderToLink();
    m_sender->clock();
    m_trace->clock();
    EXPECT_EQ(m_sender->state(), DataAckSenderState::ACK_TIMEOUT);
    EXPECT_EQ(m_timeout, false);
}

// U
TEST_F(DataAckSenderTest, SendLogsBusyInTimeout) {
    goToAckTimeout();
    EXPECT_EQ(m_sender->sendData(0xC9), false);
    // Log manually verified
}

// V
TEST_F(DataAckSenderTest, AckReceivedInTimeout) {
    goToAckTimeout();
    EXPECT_EQ(m_sender->_ack_rxed(), false);
    m_sender->ackReceived();
    EXPECT_EQ(m_sender->_ack_rxed(), true);
}

// W
TEST_F(DataAckSenderTest, EnterSendingAckSetsRTSClearsAckRxed) {
    m_sender->sendData(0xC9);
    constexpr int expected_bits = 11;
    constexpr int expected_samples = expected_bits * 16;
    for (int i=0; i<expected_samples - 1; i++) { // -1 so we set ack_rxed and send_ack before state change
        m_sender->clock();
        m_trace->clock();
    }
    EXPECT_EQ(queryReadyToSend(), false);
    m_sender->ackReceived();
    EXPECT_EQ(m_sender->_ack_rxed(), true);
    m_sender->sendAck();
    EXPECT_EQ(m_sender->_send_ack(), true);
    m_sender->clock();
    m_trace->clock();

    EXPECT_EQ(m_sender->state(), DataAckSenderState::SENDING_ACK);
    EXPECT_EQ(queryReadyToSend(), true);
    EXPECT_EQ(m_sender->_ack_rxed(), false);
}

// W (null safety)
TEST_F(DataAckSenderTest, EnterSendingAckDoesNotSetsRTSButClearsAckRxedNoListener) {
    m_sender->sendData(0xC9);
    constexpr int expected_bits = 11;
    constexpr int expected_samples = expected_bits * 16;
    for (int i=0; i<expected_samples - 1; i++) { // -1 so we set ack_rxed and send_ack before state change
        m_sender->clock();
        m_trace->clock();
    }
    EXPECT_EQ(queryReadyToSend(), false);
    m_sender->ackReceived();
    EXPECT_EQ(m_sender->_ack_rxed(), true);
    m_sender->sendAck();
    EXPECT_EQ(m_sender->_send_ack(), true);
    m_sender->unregisterSenderToLink();
    m_sender->clock();
    m_trace->clock();

    EXPECT_EQ(m_sender->state(), DataAckSenderState::SENDING_ACK);
    EXPECT_EQ(queryReadyToSend(), false);
    EXPECT_EQ(m_sender->_ack_rxed(), false);
}

// X
TEST_F(DataAckSenderTest, ExitSendingAckClearsSendAck) {
    m_sender->sendData(0xC9);
    constexpr int expected_bits = 11;
    constexpr int expected_samples = expected_bits * 16;
    for (int i=0; i<expected_samples - 1; i++) { // -1 so we set send_ack before state change
        m_sender->clock();
        m_trace->clock();
    }
    EXPECT_EQ(m_sender->_send_ack(), false);
    m_sender->sendAck();
    EXPECT_EQ(m_sender->_send_ack(), true);
    m_sender->clock();
    m_trace->clock();

    EXPECT_EQ(m_sender->state(), DataAckSenderState::SENDING_ACK);
    constexpr int expected_ack_bits = 2;
    constexpr int expected_ack_samples = expected_ack_bits * 16;
    for (int i=0; i<expected_ack_samples; i++) {
        m_sender->clock();
        m_trace->clock();
    }

    EXPECT_EQ(m_sender->_send_ack(), false);
}

// C, N, E
TEST_F(DataAckSenderTest, StoryNoDataEnqueuedSendingAckClocksAckOutGoesIdle) {
    m_sender->sendAck();
    EXPECT_EQ(m_sender->_data_enqueued(), false);
    constexpr int expected_bits = 2;
    constexpr int expected_samples = expected_bits * 16;
    for (int i=0; i<expected_samples; i++) {
        EXPECT_EQ(m_sender->state(), DataAckSenderState::SENDING_ACK);
        m_sender->clock();
        m_trace->clock();
    }

    // There's no data_enqueued so back to idle.
    EXPECT_EQ(m_sender->state(), DataAckSenderState::IDLE);

    // Heard our ack?
    EXPECT_EQ(m_sender->_queueLength(), 0);
    const std::vector<bool> heard = m_trace->heard();
    EXPECT_EQ(heard.size(), expected_samples);
    constexpr int expected_ack_bits[expected_bits] = { 1, 0 };
    const std::vector<bool> expected = generate_sample_vector_from_bits(expected_ack_bits, expected_bits);
    EXPECT_EQ(heard, expected);
}

// M, G (N)
TEST_F(DataAckSenderTest, StoryDataEnqueuedWhenSendingAckClocksAckOutThenGoesSendingData) {
    m_sender->sendAck();
    EXPECT_EQ(m_sender->_data_enqueued(), false);
    EXPECT_EQ(m_sender->state(), DataAckSenderState::SENDING_ACK);
    // Enqueue some data while we're sending the ack...
    EXPECT_EQ(m_sender->sendData(0xC9), true);
    EXPECT_EQ(m_sender->_data_enqueued(), true);
    EXPECT_EQ(m_sender->_data_enqueued_buffer(), 0xC9);

    // Needs clocking to send the signals out. 16 clock pulses/bit.
    // An ack is a 1 followed by a 0.
    constexpr int expected_bits = 2;
    constexpr int expected_samples = expected_bits * 16;
    for (int i=0; i<expected_samples; i++) {
        EXPECT_EQ(m_sender->state(), DataAckSenderState::SENDING_ACK);
        m_sender->clock();
        m_trace->clock();
    }
    const std::vector<bool> heard = m_trace->heard();
    EXPECT_EQ(heard.size(), expected_samples);
    constexpr int expected_ack_bits[expected_bits] = { 1, 0 };
    const std::vector<bool> expected = generate_sample_vector_from_bits(expected_ack_bits, expected_bits);
    EXPECT_EQ(heard, expected);

    // There's data_enqueued so we should be ready to send it.
    EXPECT_EQ(m_sender->state(), DataAckSenderState::SENDING_DATA);
    EXPECT_EQ(m_sender->_queueLength(), 11);
    EXPECT_EQ(m_sender->_data(), 0x0327); // Data is two start bits (1), the data bits, then a stop bit (0).
    EXPECT_EQ(m_sender->_ack_rxed(), false);
    EXPECT_EQ(m_sender->_data_enqueued(), false); // These get reset after we've transitioned.
    EXPECT_EQ(m_sender->_data_enqueued_buffer(), 0x00);
}

// B, Q, I
TEST_F(DataAckSenderTest, StoryDataSentButNoAckReceived) {
    setReadyToSend();
    EXPECT_EQ(m_sender->state(), DataAckSenderState::IDLE);

    EXPECT_EQ(m_sender->sendData(0xC9), true); // Needs clocking to send the signals out. 16 clock pulses/bit.

    EXPECT_EQ(queryReadyToSend(), false); // We're sending now, so disallow further until it's acked.
    // Data is two start bits (1), the data bits, then a stop bit (0).
    constexpr int expected_bits = 11;
    constexpr int expected_samples = expected_bits * 16;
    EXPECT_EQ(m_sender->state(), DataAckSenderState::SENDING_DATA);
    EXPECT_EQ(m_sender->_queueLength(), expected_bits);
    EXPECT_EQ(m_sender->_data(), 0x0327);
    EXPECT_EQ(m_sender->_ack_rxed(), false);
    logDebug("Data to be clocked out looks right with framing");
    for (int i=0; i<expected_samples; i++) {
        EXPECT_EQ(m_sender->state(), DataAckSenderState::SENDING_DATA);
        m_sender->clock();
        m_trace->clock();
    }
    EXPECT_EQ(m_sender->state(), DataAckSenderState::ACK_TIMEOUT);
    EXPECT_EQ(m_sender->_queueLength(), 0);
    const std::vector<bool> heard = m_trace->heard();
    EXPECT_EQ(heard.size(), expected_samples);   //       vvv note LSB first vvv
    const int expected_data_bits[expected_bits] = { 1, 1,  1, 0, 0, 1, 0, 0, 1, 1,  0 };
    const std::vector<bool> expected = generate_sample_vector_from_bits(expected_data_bits, expected_bits);
    EXPECT_EQ(heard, expected);
    EXPECT_EQ(m_sender->_ack_rxed(), false);
}


// B, Q, R, A, K
TEST_F(DataAckSenderTest, StoryDataSentAndAckReceived) {
    setReadyToSend();
    EXPECT_EQ(m_sender->state(), DataAckSenderState::IDLE);

    EXPECT_EQ(m_sender->sendData(0xC9), true); // Needs clocking to send the signals out. 16 clock pulses/bit.

    // Data is two start bits (1), the data bits, then a stop bit (0).
    constexpr int expected_bits = 11;
    constexpr int expected_samples = expected_bits * 16;
    EXPECT_EQ(m_sender->state(), DataAckSenderState::SENDING_DATA);
    EXPECT_EQ(m_sender->_queueLength(), expected_bits);
    EXPECT_EQ(m_sender->_data(), 0x0327);
    EXPECT_EQ(m_sender->_ack_rxed(), false);
    logDebug("Data to be clocked out looks right with framing");
    for (int i=0; i<expected_samples - 1; i++) { // -1 to sense ack_rxed flag before going IDLE and resetting
        EXPECT_EQ(m_sender->state(), DataAckSenderState::SENDING_DATA);
        m_sender->clock();
        m_trace->clock();

        // Ack would be signalled after the receiver has detected the second start bit,
        // so after 16 (first bit) + 9 (end of majority detection of second bit) = 25.
        if (i == 25) {
            logDebug("Pretend an ack has been seen");
            m_sender->ackReceived();
        }
    }
    // Just before going idle, has the sender seen that ack?
    EXPECT_EQ(m_sender->_ack_rxed(), true);
    // RTS should be clear (we're sending)
    EXPECT_EQ(m_ready_to_send, false);
    // Perform more clocking...
    m_sender->clock();
    m_trace->clock();

    EXPECT_EQ(m_sender->state(), DataAckSenderState::IDLE);
    EXPECT_EQ(m_sender->_queueLength(), 0);
    EXPECT_EQ(m_sender->_ack_rxed(), false);
    // And the AsyncLink can set its ready-to-send flag.
    EXPECT_EQ(m_ready_to_send, true);
    const std::vector<bool> heard = m_trace->heard();
    EXPECT_EQ(heard.size(), expected_samples);   //       vvv note LSB first vvv
    const int expected_data_bits[expected_bits] = { 1, 1,  1, 0, 0, 1, 0, 0, 1, 1,  0 };
    const std::vector<bool> expected = generate_sample_vector_from_bits(expected_data_bits, expected_bits);
    EXPECT_EQ(heard, expected);
}


// H, J
TEST_F(DataAckSenderTest, StoryDataSendThenSendAckThenTimeout) {
    EXPECT_EQ(m_sender->sendData(0xC9), true);
    constexpr int num_expected_data_bits = 11;
    constexpr int num_expected_data_samples = num_expected_data_bits * 16;
    EXPECT_EQ(m_sender->state(), DataAckSenderState::SENDING_DATA);
    logDebug("Data to be clocked out looks right with framing");
    for (int i=0; i<num_expected_data_samples - 1; i++) { // -1 to sense send_ack flag before leaving SENDING_DATA
        m_sender->clock();
        m_trace->clock();
        // If the receiver is receiving data, and asks the sender to send an ack.
        // Ack would be requested after the receiver has detected the second start bit,
        // so after 16 (first bit) + 9 (end of majority detection of second bit) = 25.
        if (i == 25) {
            logDebug("Please ack imaginary received data");
            m_sender->sendAck();
        }
    }
    EXPECT_EQ(m_sender->state(), DataAckSenderState::SENDING_DATA);
    // Has the sender registered it needs to send the ack?
    EXPECT_EQ(m_sender->_send_ack(), true);
    // We have not heard an ack.
    EXPECT_EQ(m_sender->_ack_rxed(), false);
    // A bit more clocking...
    m_sender->clock();
    m_trace->clock();

    logInfoF("the state is now %s", DataAckSenderStateToString(m_sender->state()));
    EXPECT_EQ(m_sender->state(), DataAckSenderState::SENDING_ACK);
    // Still recall you were told to send the ack?
    EXPECT_EQ(m_sender->_send_ack(), true);
    constexpr int num_expected_ack_bits = 2;
    constexpr int num_expected_ack_samples = num_expected_ack_bits * 16;
    EXPECT_EQ(m_sender->_queueLength(), num_expected_ack_bits);
    EXPECT_EQ(m_sender->_data(), 0x01);
    for (int i=0; i<num_expected_ack_samples; i++) {
        EXPECT_EQ(m_sender->state(), DataAckSenderState::SENDING_ACK);
        m_sender->clock();
        m_trace->clock();
    }

    const std::vector<bool> heard = m_trace->heard();
    constexpr int num_expected_data_ack_bits = num_expected_data_bits + num_expected_ack_bits;
    constexpr int num_expected_data_ack_samples = num_expected_data_ack_bits * 16;
    EXPECT_EQ(heard.size(), num_expected_data_ack_samples); //        vvv note LSB first vvv        ack
    const int expected_bits[num_expected_data_ack_bits] = { 1, 1,  1, 0, 0, 1, 0, 0, 1, 1,  0,     1, 0 };
    const std::vector<bool> expected = generate_sample_vector_from_bits(expected_bits, num_expected_data_ack_bits);
    EXPECT_EQ(heard, expected);
    logInfoF("the state is now %s and send_ack is %d", DataAckSenderStateToString(m_sender->state()), m_sender->_send_ack());

    // send_ack is cleared upon exiting SENDING_ACK
    EXPECT_EQ(m_sender->_send_ack(), false);

    EXPECT_EQ(m_sender->state(), DataAckSenderState::ACK_TIMEOUT);
}


// TODO how to signal you can't ack or send data in a non-idle state


// Single-letter comments relate the test to the labelled transitions on the statechart.
// Lower case letters are not implemented / covered by tests yet.
// ABCDEFGHIJKL
class DataAckReceiverTest : public ::testing::Test, ReceiverToSender, ReceiverToLink {
protected:

    void SetUp() override {
        setLogLevel(LOGLEVEL_DEBUG);
        logDebug("SetUp start");
        m_receiver = new DataAckReceiver(0);
        m_receiver->registerReceiverToLink(*this); // dereference this to get a reference
        m_receiver->registerReceiverToSender(*this);
        logDebug("Setup complete");
        logFlush();
    }

    void TearDown() override {
        logDebug("TearDown start");
        logDebug("TearDown complete");
        logFlush();
    }

    // ReceiverToSender
    void sendAck() override {
        logDebug("Send Ack requested");
        m_send_acks_received++;
    }

    void ackReceived() override {
        logDebug("Ack received");
        m_acks_received++;
    }

    // ReceiverToLink
    void framingError() override {
        logWarn("Framing error");
        m_framing_errors_received++;
    }

    void overrunError() override {
        logWarn("Overrun error");
        m_overrun_errors_received++;
    }

    void dataReceived(const BYTE8 data) override {
        logInfoF("Data received: 0b%s", byte_to_binary(data));
        m_data = data;
        m_data_received++;
        logInfo("Setting read data available");
        m_read_data_available = true;
    }

    bool queryReadDataAvailable() override {
        logInfoF("Query read data available %d", m_read_data_available);
        return m_read_data_available;
    }

    void clearReadDataAvailable() override {
        logInfo("Clear read data available");
        m_read_data_available = false;
    }


    void goToStartBit2() const {
        // Enter START_BIT_2
        m_receiver->bitStateReceived(true);
        EXPECT_EQ(m_receiver->state(), DataAckReceiverState::START_BIT_2);
    }

    void goToData() {
        goToStartBit2();
        // Enter DATA
        m_read_data_available = false;
        m_receiver->bitStateReceived(true);
        EXPECT_EQ(m_receiver->state(), DataAckReceiverState::DATA);
    }

    void goToDiscard() {
        goToStartBit2();
        // Enter DISCARD
        m_read_data_available = true;
        m_receiver->bitStateReceived(true);
        EXPECT_EQ(m_receiver->state(), DataAckReceiverState::DISCARD);
    }

    void goToStopBit() {
        goToData();

        m_receiver->bitStateReceived(true);
        m_receiver->bitStateReceived(true);
        m_receiver->bitStateReceived(false);
        m_receiver->bitStateReceived(false);
        m_receiver->bitStateReceived(false);
        m_receiver->bitStateReceived(false);
        m_receiver->bitStateReceived(true);
        m_receiver->bitStateReceived(true);
        EXPECT_EQ(m_receiver->state(), DataAckReceiverState::STOP_BIT);
    }

    DataAckReceiver *m_receiver = nullptr;
    int m_acks_received = 0;
    int m_send_acks_received = 0;
    int m_framing_errors_received = 0;
    int m_overrun_errors_received = 0;
    int m_data_received = 0;
    BYTE8 m_data = 0;
    bool m_read_data_available = false;
};

TEST_F(DataAckReceiverTest, InitialConditions) {
    EXPECT_EQ(m_receiver->state(), DataAckReceiverState::IDLE);
    EXPECT_EQ(m_receiver->_bit_count(), -1);
    EXPECT_EQ(m_receiver->_buffer(), 0x69);

    EXPECT_EQ(m_acks_received, 0);
    EXPECT_EQ(m_send_acks_received, 0);
    EXPECT_EQ(m_framing_errors_received, 0);
    EXPECT_EQ(m_data_received, 0);
    EXPECT_EQ(m_data, 0);
    EXPECT_EQ(m_read_data_available, false); // on registration
}

// A
TEST_F(DataAckReceiverTest, IdleReceivesHighGoesToStartBit2) {
    m_receiver->bitStateReceived(true);
    EXPECT_EQ(m_receiver->state(), DataAckReceiverState::START_BIT_2);
}

// B
TEST_F(DataAckReceiverTest, StartBit2ReceivesLowAcksGoesToIdle) {
    goToStartBit2();

    // Back to IDLE - that should have generated an ack; our callback
    // would have been called, incrementing the count.
    m_receiver->bitStateReceived(false);

    EXPECT_EQ(m_receiver->state(), DataAckReceiverState::IDLE);
    EXPECT_EQ(m_acks_received, 1);
}

// B (null safety)
TEST_F(DataAckReceiverTest, StartBit2ReceivesLowNoAckReceiverGoesToIdle) {
    // Knock out the listener (the test setup always installs one)
    // Test that the listener is only called if it's not nullptr.
    // Without that check, this test crashes.
    m_receiver->unregisterReceiverToSender();
    goToStartBit2();

    // Back to IDLE - that should have generated an ack; our callback
    // would have been called, if we had one.
    m_receiver->bitStateReceived(false);

    EXPECT_EQ(m_receiver->state(), DataAckReceiverState::IDLE);
    EXPECT_EQ(m_acks_received, 0); // no ack listener
}

// D, F
TEST_F(DataAckReceiverTest, StartBit2ReceivesHighRDAFalseGoesToData) {
    goToStartBit2();

    // The receiver will query the Read Data Available which is false (space in
    // receiver buffer). It will call the 'send ack' callback, notifying
    // the DataAckSender to send an ack as we can receive this data.
    m_read_data_available = false;
    m_receiver->bitStateReceived(true);

    EXPECT_EQ(m_receiver->state(), DataAckReceiverState::DATA);
    EXPECT_EQ(m_send_acks_received, 1);
    EXPECT_EQ(m_receiver->_bit_count(), 0);
    EXPECT_EQ(m_receiver->_buffer(), 0x00);
}

// D (null safety #1)
TEST_F(DataAckReceiverTest, StartBit2ReceivesHighRDAFalseNoLinkToLinkGoesToDiscard) {
    // Knock out the link listener (the test setup always installs one)
    // Test that the listener is only called if it's not nullptr.
    // Without that check, this test crashes.
    m_receiver->unregisterReceiverToLink();
    goToStartBit2();

    // The receiver would call the 'query RDA' method, to see if we can receive, but we
    // don't have one, so have to assume there's no space to receive this
    // data, and no ability to ack. So, reject the data by going into
    // DISCARD.
    m_receiver->bitStateReceived(true);

    EXPECT_EQ(m_receiver->state(), DataAckReceiverState::DISCARD);
    EXPECT_EQ(m_send_acks_received, 0);
    EXPECT_EQ(m_receiver->_bit_count(), 0);
    EXPECT_EQ(m_receiver->_buffer(), 0x00);
}

// D (null safety #2)
TEST_F(DataAckReceiverTest, StartBit2ReceivesHighRDAFalseNoServerToLinkGoesToDiscard) {
    // Knock out the server listener (the test setup always installs one)
    // Test that the listener is only called if it's not nullptr.
    // Without that check, this test crashes.
    m_receiver->unregisterReceiverToSender();
    goToStartBit2();

    m_read_data_available = false;
    // The receiver would call the 'send ack' method, but we
    // don't have one, so have to assume there's no space to receive this
    // data, and no ability to ack. So, reject the data by going into
    // DISCARD.
    m_receiver->bitStateReceived(true);

    EXPECT_EQ(m_receiver->state(), DataAckReceiverState::DISCARD);
    EXPECT_EQ(m_send_acks_received, 0);
    EXPECT_EQ(m_receiver->_bit_count(), 0);
    EXPECT_EQ(m_receiver->_buffer(), 0x00);
}

// D, E
TEST_F(DataAckReceiverTest, StartBit2ReceivesHighRDATrueCallsOverrunGoesToDiscard) {
    goToStartBit2();
    m_read_data_available = true;
    // The receiver will query the Read Data Available which is true (no space in
    // receiver buffer).
    m_receiver->bitStateReceived(true);

    EXPECT_EQ(m_receiver->state(), DataAckReceiverState::DISCARD);
    EXPECT_EQ(m_send_acks_received, 0);
    EXPECT_EQ(m_receiver->_bit_count(), 0); // Discard needs this zeroing.
    EXPECT_EQ(m_receiver->_buffer(), 0x00);
}

// G, K
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

// H, L
TEST_F(DataAckReceiverTest, DiscardToIdle) {
    goToDiscard();

    m_receiver->bitStateReceived(true);
    m_receiver->bitStateReceived(true);
    m_receiver->bitStateReceived(false);
    m_receiver->bitStateReceived(false);
    m_receiver->bitStateReceived(false);
    m_receiver->bitStateReceived(false);
    m_receiver->bitStateReceived(true);
    m_receiver->bitStateReceived(true);

    // The absorbed stop bit
    m_receiver->bitStateReceived(true);

    EXPECT_EQ(m_receiver->_bit_count(), 9);
    EXPECT_EQ(m_receiver->state(), DataAckReceiverState::IDLE);
}

// I
TEST_F(DataAckReceiverTest, StopBitFramingError) {
    goToStopBit();

    // A proper stop bit is a false...
    m_receiver->bitStateReceived(true);
    EXPECT_EQ(m_framing_errors_received, 1);
    EXPECT_EQ(m_receiver->state(), DataAckReceiverState::IDLE);
}

// I (null safety)
TEST_F(DataAckReceiverTest, StopBitFramingErrorNoLinkListener) {
    goToStopBit();
    m_receiver->unregisterReceiverToLink();

    // A proper stop bit is a false...
    m_receiver->bitStateReceived(true);
    EXPECT_EQ(m_framing_errors_received, 0);
    EXPECT_EQ(m_receiver->state(), DataAckReceiverState::IDLE);
}

// J
TEST_F(DataAckReceiverTest, StopBitSendsDataGoesToIdle) {
    goToStopBit();

    // The 'data rx' cb will set RDA.
    EXPECT_EQ(m_read_data_available, false);
    // A proper stop bit is a false...
    m_receiver->bitStateReceived(false);
    EXPECT_EQ(m_data_received, 1);
    EXPECT_EQ(m_data, 0b11000011);
    EXPECT_EQ(m_receiver->state(), DataAckReceiverState::IDLE);
    EXPECT_EQ(m_read_data_available, true);
}

// J (null safety)
TEST_F(DataAckReceiverTest, StopBitDataNoListenerGoesToIdle) {
    goToStopBit();
    m_receiver->unregisterReceiverToLink();

    // A proper stop bit is a false...
    m_receiver->bitStateReceived(false);
    EXPECT_EQ(m_data_received, 0);
    EXPECT_EQ(m_receiver->state(), DataAckReceiverState::IDLE);
}

// C
TEST_F(DataAckReceiverTest, IdleLowStaysIdle) {
    EXPECT_EQ(m_receiver->state(), DataAckReceiverState::IDLE);
    m_receiver->bitStateReceived(false);
    EXPECT_EQ(m_receiver->state(), DataAckReceiverState::IDLE);
    m_receiver->bitStateReceived(false);
    EXPECT_EQ(m_receiver->state(), DataAckReceiverState::IDLE);
    m_receiver->bitStateReceived(false);
    EXPECT_EQ(m_receiver->state(), DataAckReceiverState::IDLE);
}
