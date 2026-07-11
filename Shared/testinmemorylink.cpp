//------------------------------------------------------------------------------
//
// File        : testinmemorylink.cpp
// Description : Tests for the InMemoryLink.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 08/07/2026
//
// (C) 2005-2026 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <vector>
#include <chrono>
#include <thread>

#include "gtest/gtest.h"
#include "link.h"
#include "inmemorylink.h"
#include "constants.h"
#include "log.h"

class InMemoryLinkTest : public ::testing::Test {
public:
    InMemoryLinkTest() = default;
protected:

    void SetUp() override {
        setLogLevel(LOGLEVEL_DEBUG);
        logDebug("SetUp start");

        m_linkFactory = new InMemoryLinkFactory(2, 3);
        m_linkA = dynamic_cast<InMemoryLink *>(m_linkFactory->linkA());
        m_linkA->setDebug(true);
        m_linkB = dynamic_cast<InMemoryLink *>(m_linkFactory->linkB());
        m_linkB->setDebug(true);
        logDebug("Setup complete");
        logFlush();
    }

    void TearDown() override {
        logDebug("TearDown start");
        delete m_linkA;
        delete m_linkB;
        delete m_linkFactory;
        logDebug("TearDown complete");
        logFlush();
    }

    InMemoryLinkFactory *m_linkFactory = nullptr;
    InMemoryLink *m_linkA = nullptr;
    InMemoryLink *m_linkB = nullptr;
};

TEST_F(InMemoryLinkTest, InitialConditions) {
    logDebug("InitialConditions start");
    EXPECT_EQ(m_linkA->getLinkNo(), 2);
    EXPECT_EQ(m_linkB->getLinkNo(), 3);
    EXPECT_EQ(m_linkA->_readAvailable(), false);
    EXPECT_EQ(m_linkA->_writeAvailable(), true);
    EXPECT_EQ(m_linkB->_readAvailable(), false);
    EXPECT_EQ(m_linkB->_writeAvailable(), true);
    logDebug("InitialConditions end");
}

TEST_F(InMemoryLinkTest, WriteReadTransitionConditionsAtoB) {
    EXPECT_EQ(m_linkA->_writeAvailable(), true);
    EXPECT_EQ(m_linkB->_readAvailable(), false);

    m_linkA->writeByte(0x42);
    EXPECT_EQ(m_linkA->_writeAvailable(), false);
    EXPECT_EQ(m_linkB->_readAvailable(), true);

    EXPECT_EQ(m_linkB->readByte(), 0x42);
    EXPECT_EQ(m_linkA->_writeAvailable(), true);
    EXPECT_EQ(m_linkB->_readAvailable(), false);
}

TEST_F(InMemoryLinkTest, WriteReadTransitionConditionsBtoA) {
    EXPECT_EQ(m_linkA->_readAvailable(), false);
    EXPECT_EQ(m_linkB->_writeAvailable(), true);

    m_linkB->writeByte(0x42);
    EXPECT_EQ(m_linkA->_readAvailable(), true);
    EXPECT_EQ(m_linkB->_writeAvailable(), false);

    EXPECT_EQ(m_linkA->readByte(), 0x42);
    EXPECT_EQ(m_linkA->_readAvailable(), false);
    EXPECT_EQ(m_linkB->_writeAvailable(), true);
}

TEST_F(InMemoryLinkTest, WriteReadTransitionConditionsBoth) {
    EXPECT_EQ(m_linkA->_readAvailable(), false);
    EXPECT_EQ(m_linkB->_readAvailable(), false);
    EXPECT_EQ(m_linkA->_writeAvailable(), true);
    EXPECT_EQ(m_linkB->_writeAvailable(), true);

    m_linkA->writeByte(0x01);
    m_linkB->writeByte(0x02);
    EXPECT_EQ(m_linkA->_readAvailable(), true);
    EXPECT_EQ(m_linkB->_readAvailable(), true);
    EXPECT_EQ(m_linkA->_writeAvailable(), false);
    EXPECT_EQ(m_linkB->_writeAvailable(), false);

    EXPECT_EQ(m_linkA->readByte(), 0x02);
    EXPECT_EQ(m_linkB->readByte(), 0x01);
    EXPECT_EQ(m_linkA->_readAvailable(), false);
    EXPECT_EQ(m_linkB->_readAvailable(), false);
    EXPECT_EQ(m_linkA->_writeAvailable(), true);
    EXPECT_EQ(m_linkB->_writeAvailable(), true);
}

TEST_F(InMemoryLinkTest, WriteByteAndRead) {
    logDebug("WriteByteAndRead start");
    for (int i=0x00; i<=0xFF; i++) {
        auto val = static_cast<BYTE8>(i);
        m_linkA->writeByte(val);
        EXPECT_EQ(val, m_linkB->readByte());

        BYTE8 inverse = 0xFF-val;
        m_linkB->writeByte(inverse);
        EXPECT_EQ(inverse, m_linkA->readByte());
    }
    logDebug("WriteByteAndRead end");
}

TEST_F(InMemoryLinkTest, WriteByteAndReadThreadedTortureTest) {
    logDebug("WriteByteAndReadThreadedTortureTest start");
    std::thread *a_thread = nullptr;
    std::thread *b_thread = nullptr;
    a_thread = new std::thread([this] {
        for (int i=0x00; i<=0xFF; i++) {
            auto val = static_cast<BYTE8>(i);
            m_linkA->writeByte(val);
            std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10));
        }
        for (int i=0x00; i<=0xFF; i++) {
            auto val = static_cast<BYTE8>(i);
            EXPECT_EQ(val, m_linkA->readByte());
            std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10));
        }
    });
    b_thread = new std::thread([this] {
        for (int i=0x00; i<=0xFF; i++) {
            auto val = static_cast<BYTE8>(i);
            EXPECT_EQ(val, m_linkB->readByte());
            std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10));
        }
        for (int i=0x00; i<=0xFF; i++) {
            auto val = static_cast<BYTE8>(i);
            m_linkB->writeByte(val);
            std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10));
        }
    });
    a_thread->join();
    delete a_thread;
    b_thread->join();
    delete b_thread;
    logDebug("WriteByteAndReadThreadedTortureTest end");
}
