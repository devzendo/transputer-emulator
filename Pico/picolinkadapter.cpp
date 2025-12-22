//------------------------------------------------------------------------------
//
// File        : picolinkadapter.cpp
// Description : An adapter between a Pico USB CDC link and a GPIO Asynclink.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 29/11/2025
//
// (C) 2005-2025 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include "pico/stdlib.h"
#include "pico/time.h"

#include "tusb.h"
#include "../PicoUSBCDC/cdc_app.h"
#include "log.h"
#include "gpioasynclink.h"
#include "constants.h"

using namespace std;

constexpr int CLOCK_PIN = 0;
constexpr int TX_PIN = 2;
constexpr int RX_PIN = 3;
constexpr int LED_PIN = 25;

const WORD32 WPTR = 0xCAFEF00D;


[[noreturn]] int main() {
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    usb_cdc_initialise();

    GPIOTxRxPin txRxPin = GPIOTxRxPin(TX_PIN, RX_PIN);

    auto *linkA = new GPIOAsyncLink(0, false, txRxPin);
    linkA->setDebug(true);
    linkA->initialise();

    MultipleTickHandler handler;
    handler.addLink(linkA);

    AsyncLinkClock clock(CLOCK_PIN, handler);

    clock.start();

    while (true) {
        usb_poll();

    }
}
