//------------------------------------------------------------------------------
//
// File        : testlink.cpp
// Description : Tests for link abstraction
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 19/03/2019
//
// (C) 2005-2026 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <thread>
#include <atomic>

#include "gtest/gtest.h"
#include "link.h"
#include "linkfactory.h"
#include "inmemorylink.h"
#include "gpioasynclink.h"
#include "asynclinkfixture.h"
#include "log.h"

// Note that links are currently blocking, and synchronous. If more than PIPE_BUF bytes are written, write() will block
// - see man 7 pipe.

// Optional, holds all the state needed to support the pair of links. Must be subclassed to add anything.
class LinkPairFixture {
public:
    virtual ~LinkPairFixture() = default;

    virtual void start() = 0;
    virtual void stop() =0;
};

// A pair of links, and optional fixture state.
class LinkPair {
public:
    LinkPair(Link * f, Link * s): first(f), second(s), fixture(nullptr) {};
    ~LinkPair() {
        // The test TearDown will delete the links, but not know about the
        // Fixture, so delete that here.
        logDebug("~LinkPair deleting LinkPairFixture");
        delete fixture;
        logDebug("~LinkPair deleted LinkPairFixture");
    };

    // Two-stage construction, as a fixture needs to be set in the pair
    void setFixture(LinkPairFixture * f) {
        fixture = f;
    }

    void start() const {
        if (fixture != nullptr) {
            logDebug("Starting the fixture");
            fixture->start();
            logDebug("Started the fixture");
        }
    };

    void stop() const {
        if (fixture != nullptr) {
            logDebug("Stopping the fixture");
            fixture->stop();
            logDebug("Stopping the fixture");
        }
    }
    Link * first;
    Link * second;
    LinkPairFixture * fixture;
};

typedef LinkPair* FactoryFunc();


// Holds all the state needed for a test.
class LinkPairTest : public ::testing::TestWithParam<FactoryFunc*> {
public:
    virtual ~LinkPairTest() { delete pair;}

protected:
    void SetUp() override {
        logDebug("LinkPairTest SetUp");
        pair = (*GetParam())();
        cpuLink = pair->first;
        serverLink = pair->second;

        logDebug("Starting the linkpair");
        pair->start();
        logDebug("Started the linkpair");
    }

    void TearDown() override {
        logDebug("LinkPairTest TearDown");
        logDebug("Stopping the linkpair");
        pair->stop();
        logDebug("Stopped the linkpair");

        logDebug("LinkPairTest Resetting CPU Link");
        cpuLink->resetLink();
        delete cpuLink;

        logDebug("LinkPairTestResetting Server Link");
        serverLink->resetLink();
        delete serverLink;

        delete pair;
        pair = nullptr;
    }

    void finished() {
        logDebug("End of test background thread");
        done.store(true, std::memory_order_release);
    }

    void waitForFinished() {
        logDebug("Waiting for end of test background thread");
        while (!done.load(std::memory_order_acquire)) {
            std::this_thread::yield();
        }
        if (m_thread != nullptr) {
            logDebug("Joining thread");
            m_thread->join();
            logDebug("Thread joined");
        }
        logDebug("Test background thread is definitely finished");
    }

    LinkPair *pair = nullptr;
    Link * cpuLink = nullptr;
    Link * serverLink = nullptr;
    std::thread *m_thread = nullptr;
    std::atomic<bool> done{false};
};

LinkPair * FactoryInMemory() {
    const auto * factory = new InMemoryLinkFactory(2, 3);

    Link * a = dynamic_cast<InMemoryLink *>(factory->linkA());
    a->setDebug(true);
    a->initialise();
    Link * b = dynamic_cast<InMemoryLink *>(factory->linkB());
    b->setDebug(true);
    b->initialise();

    return new LinkPair(a, b);
};

LinkPair * FactoryFifo() {
    auto * cpuLinkFactory = new LinkFactory(false, true);
    auto * serverLinkFactory = new LinkFactory(true, true);

    logDebug("Creating CPU Link");
    Link * cpuLink = cpuLinkFactory->createLink(0);
    cpuLink->setDebug(true);
    logDebug("Initialising CPU Link");
    cpuLink->initialise();

    logDebug("Creating Server Link");
    Link * serverLink = serverLinkFactory->createLink(0);
    serverLink->setDebug(true);
    logDebug("Initialising Server Link");
    serverLink->initialise();

    return new LinkPair(cpuLink, serverLink);
}

class ClockingFixture: public LinkPairFixture {
public:
    ClockingFixture() : LinkPairFixture(), clock(-1, linkClocker) {
        setLogLevel(LOGLEVEL_DEBUG);
        logDebug("In the ClockingFixture constructor");

        TxRxPin &pairA = pair.pairA();
        TxRxPin &pairB = pair.pairB();

        logDebugF("Creating Link A with pin 0x%08X", &pairA);
        linkA = new GPIOAsyncLink(0, false, pairA);
        linkA->setDebug(true);
        logDebugF("Initialising Link A 0x%08X", linkA);
        linkA->initialise();

        logDebugF("Creating Link B with pin 0x%08X", &pairB);
        linkB = new GPIOAsyncLink(1, false, pairB);
        linkB->setDebug(true);
        logDebug("Initialising Link B");
        linkB->initialise();

        logDebug("ClockingFixture adding links as Clockables");
        linkClocker.addClockable(linkA);
        linkClocker.addClockable(linkB);

        logDebug("ClockingFixture creating the LinkPair");
        pPair = new LinkPair(linkA, linkB);
        logDebug("ClockingFixture returning from ctor");
    }

    void start() override {
        logDebug("ClockingFixture start");
        clock.start();
        logDebug("ClockingFixture started");
    }

    void stop() override {
        logDebug("ClockingFixture stop");
        clock.stop();
        logDebug("ClockingFixture stopped");
    }

    LinkPair * getPair() const {
        return pPair;
    }

    CrosswiredTxRxPinPair pair;
    GPIOAsyncLink *linkA;
    GPIOAsyncLink *linkB;
    MultipleClockableClocker linkClocker;
    AsyncLinkClock clock;
    LinkPair *pPair;
};


LinkPair * FactoryGPIOAsyncLink() {
    setLogLevel(LOGLEVEL_DEBUG);
    logDebug("Starting FactoryGPIOAsyncLink. Fixture...");
    auto * fixture = new ClockingFixture();
    logDebug("Starting FactoryGPIOAsyncLink. LinkPair...");
    LinkPair * pair = fixture->getPair();
    pair->setFixture(fixture);
    return pair;
}


// What factories (types of links) can we construct here, so we have a standard set of tests for all links? Such links
// must be cross-wired. Certain link types can be, certain types definitely can't.
//
// The availability API is critical to the async/polling implementation.
//
// Implementation status of availability API:
// POSIX:
// fifolink       | Done (writeAvailable has issues with buffered I/O)
// ttylink        |
//
// WINDOWS:
// namedpipelink  |
// commlink       |
//
// PI PICO:
// picousbcdclink |
//
// CROSS-PLATFORM/TEST:
// inmemorylink   | Done
// gpioasynclink  | Done - needs wiring into tests when Link/AsyncLink are one.
// nulllink       | Can't test
// stublink       | Can't test
// tvslink        | Single-ended
//
//
// ttylink (posix) - a pair of pty 'master/slave' (not my terminology) can be used to cross-wire two of these, but could
// be tricky to construct, requiring the pair of 'master' halves to poll each other... this'd need a separate thread,
// doing the polling. The factory could have a start/stop method that handles the thread; the factory would need to be
// stored in LinkPairTest, with start/stop called from SetUp/TearDown. Or, could use socat?
//
// commlink (windows) - similarly, can a virtual com port be instantiated? virtual serial cable; too tricky to do.
//
// gpioasynclink - Link and AsyncLink isn't merged yet
// Note TestInMemoryLink has several availability tests that can be moved here once they're merged.
//
// picousbseriallink - hardware based; would require a real Pico connected, with a ttylink on the test runner side.
// nulllink - is single-ended, so there's no other end to sense/control.
// stublink, tvslink - can't be tested like this, it can't be cross-wired; one half is connected to the CPU, the other
// does I/O for a test (stublink) or sends a program (tvslink).

INSTANTIATE_TEST_CASE_P(
    ParameterisedLinkPairTest,
    LinkPairTest,
    testing::Values(&FactoryFifo, &FactoryInMemory, &FactoryGPIOAsyncLink));


TEST_P(LinkPairTest, CPUWriteAndReadByte) {
    logDebugF("In the test, links are 0x%08X and 0x%08X", cpuLink, serverLink);
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    m_thread = new std::thread([this] {
        logDebug("Writing to CPU link");
        cpuLink->writeByte(65);
        finished();
    });

    logDebug("Reading from server link");
    EXPECT_EQ(serverLink->readByte(), 65);
    waitForFinished();
    logDebug("End of test");
}

TEST_P(LinkPairTest, CPUWriteAndReadBytes) {
    m_thread = new std::thread([this] {
        BYTE8 writeBuf[4] = { 0xff, 0x7f, 0x60, 0x21 };
        const int bytesWritten = cpuLink->writeBytes(writeBuf, 4);
        EXPECT_EQ(bytesWritten, 4);
        finished();
    });

    BYTE8 readBuf[4];
    const int bytesRead = serverLink->readBytes(readBuf, 4);
    EXPECT_EQ(bytesRead, 4);

    EXPECT_EQ(readBuf[0], 0xff);
    EXPECT_EQ(readBuf[1], 0x7f);
    EXPECT_EQ(readBuf[2], 0x60);
    EXPECT_EQ(readBuf[3], 0x21);
    waitForFinished();
}

TEST_P(LinkPairTest, CPUWriteAndReadShort) {
    m_thread = new std::thread([this] {
        cpuLink->writeShort(0x0102);
        finished();
    });

    EXPECT_EQ(serverLink->readShort(), 0x0102);
    waitForFinished();
}

TEST_P(LinkPairTest, CPUWriteAndReadWord) {
    m_thread = new std::thread([this] {
        cpuLink->writeWord(0x01020304);
        finished();
    });

    EXPECT_EQ(serverLink->readWord(), 0x01020304);
    waitForFinished();
}

TEST_P(LinkPairTest, ServerWriteAndReadByte) {
    m_thread = new std::thread([this] {
        serverLink->writeByte(32);
        finished();
    });

    EXPECT_EQ(cpuLink->readByte(), 32);
    waitForFinished();
}

TEST_P(LinkPairTest, ServerWriteAndReadWord) {
    m_thread = new std::thread([this] {
        serverLink->writeWord(0x05060708);
        finished();
    });

    EXPECT_EQ(cpuLink->readWord(), 0x05060708);
    waitForFinished();
}

TEST_P(LinkPairTest, InitialConditions) {
    logDebug("InitialConditions start");
    EXPECT_EQ(cpuLink->readAvailable(), false);
    EXPECT_EQ(cpuLink->writeAvailable(), true);
    EXPECT_EQ(serverLink->readAvailable(), false);
    EXPECT_EQ(serverLink->writeAvailable(), true);
    logDebug("InitialConditions end");
}

/*
 * Too racy - TODO but need this.
TEST_P(LinkPairTest, WriteAvailabilityGoingFalseTransition) {
    logDebug("WriteAvailabilityGoingFalseTransition start");
    m_thread = new std::thread([this] {
        logDebug("WriteAvailabilityGoingFalseTransition writing in thread");
        cpuLink->writeByte(0x42); // even though in another thread, it blocks.. so will writeAvailable toggle back when it's finished?
        logDebug("WriteAvailabilityGoingFalseTransition finished write");
        finished();
    });
    logDebug("WriteAvailabilityGoingFalseTransition wait");
    std::this_thread::sleep_for(std::chrono::milliseconds(4)); // should be enough to have the write still be going on
    logDebug("WriteAvailabilityGoingFalseTransition wait end - is write available?");
    // racy, but nothing's reading
    EXPECT_EQ(cpuLink->writeAvailable(), false); // fails on FIFO, presumably 'cos there's an OS buffer.
    logDebug("Waiting for thread to finish");
    waitForFinished();
}
*/

/* Too racy
TEST_P(LinkPairTest, WriteAvailabilityGoingTrueTransition) {
    m_thread = new std::thread([this] {
        logDebug("Background thread starting to write");
        cpuLink->writeByte(0x42);
        logDebug("Background thread finished writing");
        finished();
        logDebug("Background thread finished");
    });
    // it'll be false now...
    logDebugF("Reading the byte on link %d", serverLink->getLinkNo());
    EXPECT_EQ(serverLink->readByte(), 0x42);
    logDebug("Finished reading the byte; bit of a sleep");
    std::this_thread::sleep_for(std::chrono::milliseconds(2)); // should be enough to have the write state making available again
    logDebug("Finished sleep");

    EXPECT_EQ(cpuLink->writeAvailable(), true);
    waitForFinished();
}
*/

// If data is received with an empty receive buffer, this should be signalled via
// readAvailable(). A subsequent read of a byte would return that.
TEST_P(LinkPairTest, ReadAvailabilityGoingTrueTransition) {
    EXPECT_EQ(cpuLink->readAvailable(), false); // reiterate for clarity
    logDebug("ReadAvailabilityGoingTrueTransition start, writing byte");
    serverLink->writeByte(69);
    logDebug("Checking read availability");
    EXPECT_EQ(cpuLink->readAvailable(), true);

    logDebug("Reading from link, should not block as there's data");
    EXPECT_EQ(cpuLink->readByte(), 69);
    EXPECT_EQ(cpuLink->readAvailable(), false); // now we're done.
}

/*
 * Too racy
TEST_P(LinkPairTest, ReadAvailabilityDoesNotGoTrueIfNothingReading) {
    m_thread = new std::thread([this] {
        logDebug("Background thread writing byte");
        serverLink->writeByte(32);
        logDebug("Background thread written byte");
        finished();
        logDebug("Background thread finished");
    });
    logDebugF("Slight sleep, then reading on link %d", cpuLink->getLinkNo());
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // should be enough!
    logDebug("End of sleep");
    // ST_READ_DATA_AVAILABLE is 0.
    // We have no buffer (no read has been issued), so the data is not held in the
    // status word; there will be a warning:
    // 'WARN  Link 0 received data with no buffer'
    EXPECT_EQ(cpuLink->readAvailable(), false);
    waitForFinished();
}
*/

/*
 * Too racy
TEST_P(LinkPairTest, ReadAvailabilityGoingFalseTransition) {
    m_thread = new std::thread([this] {
        std::this_thread::sleep_for(std::chrono::milliseconds(20)); // should be enough!
        serverLink->writeByte(32);
        finished();
    });
    // But... with GPIOAsyncLink, data written with nothing waiting will lead to
    // 'WARN  Link 0 received data with no buffer'
    // So the background thread above waits briefly, then writes. Hopefully the
    // read below will have set the buffer first.
    logDebug("Test waiting for background write");
    logDebug("Reading a byte (should be OK)");
    EXPECT_EQ(cpuLink->readByte(), 32);
    logDebug("Read availability check");
    EXPECT_EQ(cpuLink->readAvailable(), false);
    waitForFinished();
}
*/

TEST_P(LinkPairTest, WriteByteAndRead) {
    logDebug("WriteByteAndRead start");
    for (int i=0x00; i<=0xFF; i++) {
        auto val = static_cast<BYTE8>(i);
        auto *a_thread = new std::thread([this, val] {
            cpuLink->writeByte(val);
        });
        EXPECT_EQ(val, serverLink->readByte());
        a_thread->join();
        delete a_thread;

        BYTE8 inverse = 0xFF-val;
        auto *b_thread = new std::thread([this, inverse] {
            serverLink->writeByte(inverse);
        });
        EXPECT_EQ(inverse, cpuLink->readByte());
        b_thread->join();
        delete b_thread;
    }
    logDebug("WriteByteAndRead end");
}

TEST_P(LinkPairTest, WriteByteAndReadThreadedTortureTest) {
    logDebug("WriteByteAndReadThreadedTortureTest start");
    std::thread *a_thread = nullptr;
    std::thread *b_thread = nullptr;
    a_thread = new std::thread([this] {
        for (int i=0x00; i<=0xFF; i++) {
            const auto val = static_cast<BYTE8>(i);
            cpuLink->writeByte(val);
            std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10));
        }
        for (int i=0x00; i<=0xFF; i++) {
            auto val = static_cast<BYTE8>(i);
            EXPECT_EQ(val, cpuLink->readByte());
            std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10));
        }
    });
    b_thread = new std::thread([this] {
        for (int i=0x00; i<=0xFF; i++) {
            auto val = static_cast<BYTE8>(i);
            EXPECT_EQ(val, serverLink->readByte());
            std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10));
        }
        for (int i=0x00; i<=0xFF; i++) {
            const auto val = static_cast<BYTE8>(i);
            serverLink->writeByte(val);
            std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10));
        }
    });
    a_thread->join();
    delete a_thread;
    b_thread->join();
    delete b_thread;
    logDebug("WriteByteAndReadThreadedTortureTest end");
}

TEST_P(LinkPairTest, ResetLinkWhenReadingTest) {
    logDebug("ResetLinkWhenReadingTest start");
    std::atomic_bool reset_ok { false };
    auto *a_thread = new std::thread([this, &reset_ok] {
        logDebug("Reading from link");
        EXPECT_EQ(cpuLink->readByte(), 0);
        // The resetLink exits the readByte with a zero return.
        logDebug("Link read finished - ie reset");
        reset_ok.store(true);
    });
    logDebug("Waiting a little before resetting link");
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    logDebug("Resetting link");
    cpuLink->resetLink();
    logDebug("Joining thread");
    a_thread->join();
    logDebug("Thread joined");
    delete a_thread;
    EXPECT_EQ(reset_ok.load(), true);
    logDebug("ResetLinkWhenReadingTest end");
}

TEST_P(LinkPairTest, ResetLinkWhenWritingTest) {
    logDebug("ResetLinkWhenWritingTest start");
    std::atomic_bool reset_ok { false };
    auto *a_thread = new std::thread([this, &reset_ok] {
        logDebug("Writing first byte to link (it'll get acked, so read buffer will be full, this write will finish)");
        cpuLink->writeByte(1);
        logDebug("Writing second byte to link (read buffer full with 1, and not retrieved; write should block)");
        cpuLink->writeByte(2);
        // The resetLink exits the writeByte with a zero return.
        logDebug("Second link write finished (ie reset)");
        reset_ok.store(true);
    });
    logDebug("Waiting a little before resetting link");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    logDebug("Resetting link");
    cpuLink->resetLink();
    logDebug("Joining thread");
    a_thread->join();
    logDebug("Thread joined");
    delete a_thread;
    EXPECT_EQ(reset_ok.load(), true);
    logDebug("ResetLinkWhenWritingTest end");
}

// TODO async read and write tests.
// needs implementation of SystemLink read/writeAsync - needs finished availability api
