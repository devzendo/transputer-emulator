//------------------------------------------------------------------------------
//
// File        : cdc_app.h
// Description : USB CDC functions, based on CDC example by Ha Thach,
//               Jerzy Kasenberg, Angel Molina (MIT License).
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 29/11/2025
//
// (C) 2005-2026 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Ha Thach (tinyusb.org)
 * Copyright (c) 2020 Jerzy Kasenberg
 * Copyright (c) 2022 Angel Molina (angelmolinu@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef _CDC_APP_H
#define _CDC_APP_H

#include "pico/stdlib.h" // for type uint32_t

#ifdef __cplusplus
extern "C" {
#endif

/* Blink pattern
 * - 25 ms   : streaming data
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum {
    BLINK_STREAMING = 25,
    BLINK_NOT_MOUNTED = 250,
    BLINK_MOUNTED = 1000,
    BLINK_SUSPENDED = 2500,
};

extern uint32_t blink_interval_ms;

uint32_t millis_since_epoch();

void usb_cdc_initialise();
void usb_poll();

void usb_link_write(void *buf, uint32_t size);
uint32_t usb_link_read(void *buf, uint32_t size);
void usb_link_flush();
bool usb_log_device_connected();
bool usb_log_port_listening();
void usb_log_wait_for_connected();
void usb_log_write(void *buf, uint32_t size);
uint32_t usb_log_read(void *buf, uint32_t size);
void usb_log_flush();

#ifdef __cplusplus
}
#endif

#endif // _CDC_APP_H