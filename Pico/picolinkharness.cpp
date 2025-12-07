//------------------------------------------------------------------------------
//
// File        : picolinkharness.cpp
// Description : An interactive GPIO Asynclink exerciser.
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
#include "pico/time.h"
#include <cstdlib> // free
#include <locale>

#include "log.h"
#include "gpioasynclink.h"
#include "constants.h"
#include "getline.h"
#include "hexdump.h"

using namespace std;

constexpr int CLOCK_PIN = 0;
constexpr int TX_PIN = 2;
constexpr int RX_PIN = 3;
constexpr int LED_PIN = 25;

constexpr int LED_DELAY_MS = 250;

const WORD32 WPTR = 0xDEADBEEF;

void pause() {
    stdio_flush();
    sleep_ms(5);
    stdio_flush();
}

static BYTE8 kilobyte[1024];

bool parse_hex_byte(char *hexbytestring, BYTE8* ptr) {
    BYTE8 result = 0;
    for (int i = 0; i < 2; i++) {
        char c = hexbytestring[i];
        unsigned char digit;

        if (c >= '0' && c <= '9') {
            digit = c - '0';
        } else if (c >= 'a' && c <= 'f') {
            digit = c - 'a' + 10;
        } else if (c >= 'A' && c <= 'F') {
            digit = c - 'A' + 10;
        } else {
            return false;
        }

        result = (result << 4) | digit;
    }
    *ptr = result;
    return true;
}

void process_command(char *cmd, AsyncLink *link) {
    BYTE8 a1;
    BYTE8 a2;
    int xx;
    uint32_t msSinceBootAtStart;
    uint32_t msSinceBootAtEnd;

    logInfoF("You entered '%s'", cmd);

    if (strncmp(cmd, "help", 4) == 0) {
        logInfo("sb xx - sends the hex byte xx and receives it back");
        logInfo("sk    - sends 1KB (1024 bytes) and times how long it takes; receives it back");
        return;
    }

    if (strncmp(cmd, "sb", 2) == 0 && strlen(cmd) == 5) {
        if (parse_hex_byte(cmd + 3, &a1)) {
            logInfoF("Writing hex byte 0x%02x to link...", a1);
            if (!link->writeDataAsync(WPTR, &a1, 1)) {
                logWarn("Could not write data");
            } else {
                link->readDataAsync(WPTR, &a2, 1);
                bool writeDone = false;
                bool readDone = false;
                while (!writeDone && !readDone) {
                    if (!writeDone) {
                        if (link->writeComplete() != NotProcess_p) {
                            logInfo("Write completed");
                            writeDone = true;
                        }
                        if (link->getStatusWord() & ST_DATA_SENT_NOT_ACKED) {
                            logWarn("Write timed out without acknowledgement");
                            writeDone = true;
                            readDone = true;
                            // TODO reset link
                        }
                    }
                    if (!readDone) {
                        if (link->readComplete() != NotProcess_p) {
                            logInfoF("Read completed: 0x%02x '%c'", a2, isprint(a2) ? a2 : '?');
                            readDone = true;
                        }
                    }
                    pause();
                }
                logInfo("sb finished");
            }
        } else {
            logWarn("That's not a hex byte");
        }
    }


    // Send a kilobyte
    if (strncmp(cmd, "sk", 2) == 0) {
        msSinceBootAtStart = to_ms_since_boot(get_absolute_time());
        if (!link->writeDataAsync(WPTR, kilobyte, 1024)) {
            logWarn("Could not write data");
        } else {
            logInfo("Waiting for write and read to complete...");
            int read = 0;
            link->readDataAsync(WPTR, &a2, 1);
            bool writeDone = false;
            bool readDone = false;
            while (!writeDone && !readDone) {
                if (!writeDone) {
                    if (link->writeComplete() != NotProcess_p) {
                        writeDone = true;
                        msSinceBootAtEnd = to_ms_since_boot(get_absolute_time());
                        uint32_t msDuration = msSinceBootAtEnd - msSinceBootAtStart;
                        // Convert to MB/second
                        // 1024 bytes transferred in msDuration ms
                        // Bytes per second = 1024 / (msDuration / 1000)
                        // MB per second = (1024 / (msDuration / 1000)) / (1024 * 1024)
                        double mbPerSecond = (1024.0 * 1000.0) / (msDuration * 1024.0 * 1024.0);
                        printf("\nWrite completed in %lu ms: %f MB/s", msDuration, mbPerSecond);
                    }
                    if (link->getStatusWord() & ST_DATA_SENT_NOT_ACKED) {
                        logWarn("Write timed out without acknowledgement");
                        writeDone = true;
                        readDone = true;
                        // TODO reset link
                    }
                }
                if (!readDone) {
                    if (link->readComplete() != NotProcess_p) {
                        read++;
                        if (read == 1024) {
                            printf("\rRead completed                  \r\n");
                            readDone = true;
                        } else {
                            // Start the read of another byte...
                            link->readDataAsync(WPTR, &a2, 1);
                            if (read % 16 == 0) {
                                printf("\rReceived %d bytes      ", read);
                            }
                        }
                    }
                }
                pause();
            }
            logInfo("sk finished");
        }
    }

}


static volatile bool ledstate = false;
static void toggle_led() {
    if (ledstate) {
        printf("led on\r\n");
    } else {
        printf("led off\r\n");
    }
    gpio_put(LED_PIN, ledstate);
    ledstate = ! ledstate;
    sleep_ms(LED_DELAY_MS);
}

// const int64_t STDIO_TICKLE_TICK_INTERVAL_US = 100000; // 100ms
// struct repeating_timer g_stdio_timer;
// // Free function for the timer callback.
// static bool stdioTickleTimerCallback(repeating_timer_t *rt) {
//     // The Pi Pico USB stdio seems to need to be 'tickled' regularly when we enter a
//     // text input loop. Possibly due to the architecture of TinyUSB needing to poll
//     // regularly.
//     stdio_flush();
//     return true; // true to continue repeating
// }


[[noreturn]] int main() {
    // Initialise USB Serial STDIO...
    bool ok = stdio_init_all();
    setLogLevel(LOGLEVEL_DEBUG);

    // A little pause is needed to get stdio working?
    for (int i = 0; i < 20; i++) {
        stdio_flush();
        sleep_ms(LED_DELAY_MS);
    }
    logInfoF("Hello from picolinkharness.uf2 %d", ok);
    for (int i = 0; i < 1024; i++) {
        kilobyte[i] = i & 0xff;
    }

    logInfo("Initialising LED");
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    logInfo("Initialising GPIOTxRxPin");
    GPIOTxRxPin txRxPin = GPIOTxRxPin(TX_PIN, RX_PIN);

    logInfo("Initialising AsyncLink");
    auto *linkA = new GPIOAsyncLink(0, false, txRxPin);
    linkA->setDebug(true);
    logInfo("Initialising Link");
    linkA->initialise();

    logInfo("Initialising MultipleTickHandler");
    MultipleTickHandler handler;
    handler.addLink(linkA);

    logInfo("Initialising AsyncLinkClock");
    AsyncLinkClock clock(CLOCK_PIN, handler);
    logInfo("Starting clock...");
    clock.start();
    logInfo("Clock started");

    // The Pi Pico USB stdio seems to need to be 'tickled' regularly when we enter a
    // text input loop. Possibly due to the architecture of TinyUSB needing to poll
    // regularly.
    // add_repeating_timer_us(-STDIO_TICKLE_TICK_INTERVAL_US, &stdioTickleTimerCallback, nullptr, &g_stdio_timer);

    char *cmd_buf = nullptr;
    logInfo("Interactive Link Harness. Enter 'help' for commands.");
    while (true) {
        logPrompt();

        cmd_buf = getLine(true, '\r');
        // Pressing return to return the line does not get echoed, so need to emit \r\n for
        // subsequent log statement to be presented on its own line.
        stdio_puts("");
        if (cmd_buf == nullptr) {
            logWarn("getLine returned NULL");
        } else {
            process_command(cmd_buf, linkA);
            // logDebugF("command entered: 0x%08X '%s'", cmd_buf, cmd_buf);
            // hexdump((BYTE8*)cmd_buf, strlen(cmd_buf));
            free(cmd_buf);
        }
    }
}
