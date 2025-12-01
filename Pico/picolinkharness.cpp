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

using namespace std;

const int CLOCK_PIN = 1;
const int LOCK_PIN = 2;
const int GND_PIN = 3;
const int TX_PIN = 4;
const int RX_PIN = 5;
const int LED_PIN = 25;

const WORD32 WPTR = 0xDEADBEEF;

void pause() {
    sleep_ms(5);
}

static BYTE8 kilobyte[1024];

void process_command(char *cmd, AsyncLink *link) {
    BYTE8 a1;
    BYTE8 a2;
    int xx;
    uint32_t msSinceBootAtStart;
    uint32_t msSinceBootAtEnd;

    if (strncmp(cmd, "help", 4) == 0) {
        printf("sb xx - sends the hex byte xx and receives it back\r\n");
        printf("sk    - sends 1KB (1024 bytes) and times how long it takes; receives it back\r\n");
        return;
    }
    if (strncmp(cmd, "sb", 2) == 0) {
        if (sscanf(cmd, "sb %02x", &xx) == 1) {
            a1 = xx; // Byte
            if (!link->writeDataAsync(WPTR, &a1, 1)) {
                printf("Could not write data\r\n");
            } else {
                link->readDataAsync(WPTR, &a2, 1);
                bool writeDone = false;
                bool readDone = false;
                while (!writeDone && !readDone) {
                    if (!writeDone) {
                        if (link->writeComplete() != NotProcess_p) {
                            printf("Write completed\r\n");
                            writeDone = true;
                        }
                    }
                    if (!readDone) {
                        if (link->readComplete() != NotProcess_p) {
                            printf("Read completed: 0x%02x '%c'\r\n", a2, isprint(a2) ? a2 : '?');
                            readDone = true;
                        }
                    }
                    pause();
                }
                printf("sb finished\r\n");
            }
        }
        return;
    }

    // Send a kilobyte
    if (strncmp(cmd, "sk", 2) == 0) {
        msSinceBootAtStart = to_ms_since_boot(get_absolute_time());
        if (!link->writeDataAsync(WPTR, kilobyte, 1024)) {
            printf("Could not write data\r\n");
        } else {
            printf("Waiting for write and read to complete...\r\n");
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
                        printf("\nWrite completed in %lu ms: %f MB/s\r\n", msDuration, mbPerSecond);
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
            printf("sb finished\n");
        }
        return;
    }
}

int main() {
    // Initialise USB Serial STDIO...
    bool ok = stdio_init_all();
    printf("Hello from picolinkharness.uf2 %d\r\n", ok);
    for (int i = 0; i < 1024; i++) {
        kilobyte[i] = i & 0xff;
    }

    printf("Initialising LED\r\n");
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    printf("Initialising GPIOTxRxPin\r\n");
    GPIOTxRxPin txRxPin = GPIOTxRxPin(TX_PIN, RX_PIN);

    printf("Initialising AsyncLink\r\n");
    AsyncLink *linkA = new AsyncLink(0, false, txRxPin);
    linkA->setDebug(true);
    logDebug("Initialising Link");
    linkA->initialise();

    printf("Initialising MultipleTickHandler\r\n");
    MultipleTickHandler handler;
    handler.addLink(&*linkA);

    printf("Initialising AsyncLinkClock\r\n");
    AsyncLinkClock clock(CLOCK_PIN, handler);

    printf("Starting clock...\r\n");
    clock.start();
    printf("Clock started\r\n");

    char *cmd_buf = nullptr;

    printf("Interactive Link Harness. Enter 'help' for commands.\r\n");
    while (true) {
        printf("> ");
        stdio_flush();
        cmd_buf = getLine(true, '\r');
        process_command(cmd_buf, linkA);
        free(cmd_buf);
    }
}
