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
#include <cstdlib> // free

#include "log.h"
#include "asynclink.h"
#include "getline.h"

using namespace std;

const int CLOCK_PIN = 1;
const int LOCK_PIN = 2;
const int GND_PIN = 3;
const int TX_PIN = 4;
const int RX_PIN = 5;
const int LED_PIN = 25;


void process_command(char *cmd, AsyncLink *link) {

}

int main() {
    // Initialise USB Serial STDIO...
    bool ok = stdio_init_all();
    printf("Hello from picolinkharness.uf2 %d\r\n", ok);

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
