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

constexpr int CLOCK_PIN = 1;
constexpr int LOCK_PIN = 2;
constexpr int GND_PIN = 3;
constexpr int TX_PIN = 4;
constexpr int RX_PIN = 5;
constexpr int BUTTON_PIN = 6;
constexpr int LED_PIN = 25;

const WORD32 WPTR = 0xCAFEBABE;

enum class ReflectState { REFLECT, REFLECT_SLOWLY, SILENCE };
volatile ReflectState g_state = ReflectState::REFLECT;
volatile bool g_led_state = false;

// Called every LINK_CLOCK_TICK_INTERVAL_US. (500us)
// Debouncing needs to be called every checkMsec (5ms == 5000us)
// So every 10 ticks...
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
        if (m_tick_count % 10 == 0) {
            // logDebug("Tick - >> clock the Debouncer");
            if (m_debouncer != nullptr) {
                m_debouncer->debounce(gpio_get(BUTTON_PIN));
                if (m_debouncer->keyReleased) {
                    switch (g_state) {
                        case ReflectState::REFLECT:
                            g_state = ReflectState::REFLECT_SLOWLY;
                            break;
                        case ReflectState::REFLECT_SLOWLY:
                            g_state = ReflectState::SILENCE;
                            g_led_state = false;
                            gpio_put(LED_PIN, g_led_state);
                            break;
                        case ReflectState::SILENCE:
                            g_state = ReflectState::REFLECT;
                            break;
                    }
                }
            }
            // logDebug("Tick - << clock the Debouncer");
        }
        if ((g_state == ReflectState::REFLECT && m_tick_count % 500000 == 0) ||
            (g_state == ReflectState::REFLECT_SLOWLY && m_tick_count % 1000000 == 0)) {
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
    printf("Hello from picolinkreflect.uf2 %d\r\n", ok);
    // LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, false);

    printf("Initialising GPIOTxRxPin\r\n");
    GPIOTxRxPin txRxPin = GPIOTxRxPin(TX_PIN, RX_PIN);

    printf("Initialising AsyncLink\r\n");
    auto *link = new GPIOAsyncLink(0, false, txRxPin);
    link->setDebug(true);
    logDebug("Initialising Link");
    link->initialise();

    printf("Initialising Debouncer\r\n");
    Debouncer debouncer;
    printf("Initialising DebounceAndLinkTickHandler\r\n");
    DebounceAndLinkTickHandler handler;
    handler.addLink(link);
    handler.setDebouncer(&debouncer);

    printf("Initialising AsyncLinkClock\r\n");
    AsyncLinkClock clock(CLOCK_PIN, handler);

    printf("Starting clock...\r\n");
    clock.start();
    printf("Clock started\r\n");

    BYTE8 a1;
    bool readScheduled = false;
    bool writeScheduled = false;
    uint32_t write_after_time = to_ms_since_boot(get_absolute_time());
    while (true) {
        if (!readScheduled && g_state != ReflectState::SILENCE) {
            link->readDataAsync(WPTR, &a1, 1);
            readScheduled = true;
        }
        switch (g_state) {
            case ReflectState::REFLECT:
                if (writeScheduled && link->writeComplete() != NotProcess_p) {
                    writeScheduled = false; // not doing more with this atm
                }
                if (readScheduled && link->readComplete() != NotProcess_p) {
                    readScheduled = false;
                    if (!link->writeDataAsync(WPTR, &a1, 1)) {
                        printf("Could not write data\r\n");
                    } else {
                        writeScheduled = true;
                    }
                }
                break;

            case ReflectState::REFLECT_SLOWLY:
                if (writeScheduled && link->writeComplete() != NotProcess_p) {
                    writeScheduled = false; // not doing more with this atm
                }
                if (readScheduled && link->readComplete() != NotProcess_p) {
                    write_after_time = to_ms_since_boot(get_absolute_time()) + (get_rand_32() & 0x7F);
                }
                if (to_ms_since_boot(get_absolute_time()) > write_after_time) {
                    readScheduled = false;
                    if (!link->writeDataAsync(WPTR, &a1, 1)) {
                        printf("Could not write data\r\n");
                    } else {
                        writeScheduled = true;
                    }
                }

                break;
            case ReflectState::SILENCE:
                break;
        }
    }
}
