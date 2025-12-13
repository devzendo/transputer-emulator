//------------------------------------------------------------------------------
//
// File        : picolinkreflect.cpp
// Description : Reflects data sent by the interactive GPIO Asynclink exerciser.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 26/11/2025
//
// (C) 2005-2025 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <stdio.h> // Pico USB Serial STDIO
#include "pico/stdlib.h"
#include "pico/rand.h"

#include "../Shared/gpioasynclink.h"
#include "constants.h"
#include "debouncer.h"
#include "log.h"

using namespace std;

constexpr int CLOCK_PIN = 0;
constexpr int TX_PIN = 2;
constexpr int RX_PIN = 3;
constexpr int BUTTON_PIN = 4;
constexpr int LED_PIN = 25;

constexpr int LED_DELAY_MS = 250;

const WORD32 WPTR = 0xCAFEBABE;

enum class ReflectState { REFLECT, REFLECT_SLOWLY, SILENCE };
volatile ReflectState g_state = ReflectState::REFLECT;
volatile bool g_led_state = false;
volatile int g_rw_state = 0;

// Called every LINK_CLOCK_TICK_INTERVAL_US. (50us)
// Debouncing needs to be called every checkMsec (5ms == 5000us)
// So every 100 ticks...
// LED will flash every 500ms (REFLECT), 1000ms (REFLECT_SLOWLY) or not at all (SILENCE).
// So 500000us, 1000000us or 0.
class DebounceAndLinkTickHandler: public TickHandler {
public:
    DebounceAndLinkTickHandler() {
        m_tick_count = 0;
        m_debouncer = nullptr;
    }
    void addLink(AsyncLink* link) {
        m_links.push_back(link);
    }
    void setDebouncer(Debouncer* debouncer) {
        m_debouncer = debouncer;
    }
    // TickHandler
    void tick() override {
        m_tick_count++;

        // logDebug("Tick - >> clock the Links");
        for (AsyncLink* link: m_links) {
            // logDebugF("Tick - link at 0x%x", link);
            link->clock();
        }
        if (m_tick_count % 100 == 0) {
            // logDebug("Tick - >> clock the Debouncer");
            if (m_debouncer != nullptr) {
                m_debouncer->debounce(gpio_get(BUTTON_PIN));
                if (m_debouncer->keyChanged && !m_debouncer->keyReleased) {
                    //logDebug("Button released");
                    g_rw_state = 0;
                    switch (g_state) {
                        case ReflectState::REFLECT:
                            g_state = ReflectState::REFLECT_SLOWLY;
                            logDebug("Slowly reflecting");
                            break;
                        case ReflectState::REFLECT_SLOWLY:
                            g_state = ReflectState::SILENCE;
                            logDebug("Link silence");
                            g_led_state = false;
                            gpio_put(LED_PIN, g_led_state);
                            break;
                        case ReflectState::SILENCE:
                            g_state = ReflectState::REFLECT;
                            logDebug("Reflecting");
                            break;
                    }
                }
            }
            // logDebug("Tick - << clock the Debouncer");
        }
        if ((g_state == ReflectState::REFLECT && m_tick_count % 10000 == 0) ||
            (g_state == ReflectState::REFLECT_SLOWLY && m_tick_count % 20000 == 0)) {
            g_led_state = ! g_led_state;
            gpio_put(LED_PIN, g_led_state);
        }

    }
private:
    uint32_t m_tick_count;
    std::vector<AsyncLink*> m_links;
    Debouncer* m_debouncer;
};

[[noreturn]] int main() {
    // Initialise USB Serial STDIO...
    bool ok = stdio_init_all();
    setLogLevel(LOGLEVEL_DEBUG);

    // A little pause is needed to get stdio working?
    for (int i = 0; i < 20; i++) {
        stdio_flush();
        sleep_ms(LED_DELAY_MS);
    }

    logInfoF("Hello from picolinkreflect.uf2 %d", ok);
    // LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, false);

    // Button
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_down(BUTTON_PIN);

    logInfo("Initialising GPIOTxRxPin");
    GPIOTxRxPin txRxPin = GPIOTxRxPin(TX_PIN, RX_PIN);

    logInfo("Initialising AsyncLink");
    auto *link = new GPIOAsyncLink(0, false, txRxPin);
    link->setDebug(true);
    logDebug("Initialising Link");
    link->initialise();

    logInfo("Initialising Debouncer");
    Debouncer debouncer;
    logInfo("Initialising DebounceAndLinkTickHandler");
    DebounceAndLinkTickHandler handler;
    handler.addLink(link);
    handler.setDebouncer(&debouncer);

    logInfo("Initialising AsyncLinkClock");
    AsyncLinkClock clock(CLOCK_PIN, handler);

    logInfo("Starting clock...");
    clock.start();
    logInfo("Clock started");

    BYTE8 a1;
    g_rw_state = 0;

    uint32_t write_after_time = to_ms_since_boot(get_absolute_time());
    while (true) {
        switch (g_state) {
            case ReflectState::REFLECT:
                switch (g_rw_state) {
                    case 0: // initial
                        link->readDataAsync(WPTR, &a1, 1);
                        g_rw_state = 1; // wait for read to complete
                        break;
                    case 1: // wait for read to complete
                        if (link->readComplete() != NotProcess_p) {
                            link->readDataAsync(WPTR, &a1, 1);
                            if (!link->writeDataAsync(WPTR, &a1, 1)) {
                                logInfo("Could not write data; resetting...");
                                link->resetLink();
                            } else {
                                g_rw_state = 2;
                            }
                        }
                        break;
                    case 2: // wait for write to complete
                        if (link->writeComplete() != NotProcess_p) {
                            g_rw_state = 1; // back to schedule read
                        }
                        break;
                }
                break;

            case ReflectState::REFLECT_SLOWLY:
                switch (g_rw_state) {
                    case 0: // initial
                        link->readDataAsync(WPTR, &a1, 1);
                        g_rw_state = 1; // wait for read to complete
                        break;
                    case 1: // wait for read to complete
                        if (link->readComplete() != NotProcess_p) {
                            write_after_time = to_ms_since_boot(get_absolute_time()) + (get_rand_32() & 0x7F);
                            g_rw_state = 2;
                        }
                        break;
                    case 2:
                        if (to_ms_since_boot(get_absolute_time()) > write_after_time) {
                            if (!link->writeDataAsync(WPTR, &a1, 1)) {
                                logInfo("Could not write data; resetting...");
                                link->resetLink();
                            } else {
                                g_rw_state = 3;
                            }
                        }
                        break;
                    case 3: // wait for write to complete
                        if (link->writeComplete() != NotProcess_p) {
                            g_rw_state = 0; // back to schedule read
                        }
                        break;
                }
                break;

            case ReflectState::SILENCE:
                sleep_ms(5);
                break;
        }
    }
}
