//------------------------------------------------------------------------------
//
// File        : cdc_app.c
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

#include "bsp/board_api.h"
#include "tusb.h"
// #include "pico/stdlib.h" // needed by tinyusb
#include "cdc_app.h"

#include <sys/unistd.h>

// These tie up to the interface numbers in usb_descriptors.c.
const uint8_t LINK_ITF = 0;
const uint8_t LOG_ITF = 1;

// I see DRxC,K after issuing the 'knock'.
// (DTR. RTS, coding bit rate nonzero), knock
static char knock_events[64] = "\0";

void add_knock(char c) {
    int l = strlen(knock_events);
    if (l < 63) {
        knock_events[l++] = c;
        knock_events[l] = '\0';
    }
}
enum KnockState { WaitingForConnection, Connected, Knocked };
static enum KnockState knock_state = WaitingForConnection;

// Invoked when cdc when line state changed e.g connected/disconnected
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts) {
cdc_line_coding_t coding;
    add_knock(dtr ? 'D' : 'd');
    add_knock(rts ? 'R' : 'r');
    tud_cdc_n_get_line_coding(itf, &coding);
    add_knock(coding.bit_rate == 0 ? '0' : 'x');

    if (dtr) {
        // Terminal connected
        if (itf == LOG_ITF && knock_state == WaitingForConnection) {
            knock_state = Connected;
            add_knock('C');
        }
    } else {
        // Terminal disconnected
        knock_state = WaitingForConnection;
        add_knock('X');

        // touch1200 only with first CDC instance (Serial)
        if (itf == 0) {
            tud_cdc_get_line_coding(&coding);
            if (coding.bit_rate == 1200) {
                if (board_reset_to_bootloader) {
                    board_reset_to_bootloader();
                }
            }
        }
    }
    add_knock(',');
}

// Invoked when CDC interface received data from host
void tud_cdc_rx_cb(uint8_t itf) {
    uint8_t buf[64];
    uint8_t hdr[32];
    uint32_t count;

    // connected() check for DTR bit
    // Most but not all terminal client set this when making connection
    if (tud_cdc_n_connected(itf)) {
        if (tud_cdc_n_available(itf)) { // data is available
            if (itf == LOG_ITF) {
                if (knock_state == Connected) {
                    count = tud_cdc_n_read(itf, buf, 1);
                    if (count == 1 && (buf[0] == '\r' || buf[0] == '\n')) {
                        add_knock('K');
                        tud_cdc_n_write(itf, "\r\n=== KNOCK ===\r\n", 17);
                        tud_cdc_n_write(itf, knock_events, strlen(knock_events));
                        tud_cdc_n_write(itf, "\r\n", 2);
                        tud_cdc_n_write_flush(itf);
                        knock_state = Knocked;
                    }
                } else {
                    count = tud_cdc_n_read(itf, buf, sizeof(buf));
                    // TODO pass this read data to a registered callback for this interface
                    (void) count;

                    sprintf((char * restrict) &hdr, "/dev/ttyACM%d\r\n", itf);
                    tud_cdc_n_write(itf, hdr, strlen(hdr));
                    tud_cdc_n_write(itf, buf, count); // TODO this is how you write data back to the host on this interface
                    tud_cdc_n_write_flush(itf);
                    // dummy code to check that cdc serial is responding
                    // board_led_write(0);
                    // board_delay(50);
                    // board_led_write(1);
                    // board_delay(50);
                    // board_led_write(0);
                }
            }
        }
    }
}

uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

uint32_t millis_since_epoch() {
    return board_millis();
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task() {
    static uint32_t start_ms = 0;
    static bool led_state = false;

    // Blink every interval ms
    if (board_millis() - start_ms < blink_interval_ms) return;
    start_ms += blink_interval_ms;

    board_led_write(led_state);
    led_state = 1 - led_state;
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb() {
    blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb() {
    blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en) {
    (void)remote_wakeup_en;
}

// Invoked when usb bus is resumed
void tud_resume_cb() {
}

void usb_cdc_initialise() {
    board_init();

    // init device stack on configured roothub port
    tud_init(BOARD_TUD_RHPORT);
}

void usb_poll() {
    tud_task(); // TinyUSB device task
    led_blinking_task();
}

void _usb_write(uint8_t itf, void *buf, uint32_t size) {
    void *cbuf = buf;
    usb_poll();
    // Potential of infinite loops here.
    while (size != 0) {
        uint32_t written = tud_cdc_n_write(itf, cbuf, size);
        usb_poll();
        if (written == 0) {
            // What else can we do? Infinite loop?
            board_delay(50);
            usb_poll();
        } else {
            size -= written;
            cbuf += written;
        }
    }
    usb_poll();
}

void _usb_read(uint8_t itf, void *buf, uint32_t size) {
    void *cbuf = buf;
    while (size > 0) {
        usb_poll();
        uint32_t cread = tud_cdc_n_read(itf, cbuf, size);
        usb_poll();
        if (cread == 0) {
            // What else can we do? Infinite loop?
            board_delay(50);
            usb_poll();
        } else {
            size -= cread;
            cbuf += cread;
        }
    }
}

void usb_link_write(void *buf, uint32_t size) {
    _usb_write(LINK_ITF, buf, size);
}

uint32_t usb_link_read(void *buf, uint32_t size) {
    _usb_read(LINK_ITF, buf, size);
}

void usb_link_flush() {
    tud_cdc_n_write_flush(LINK_ITF);
    usb_poll();
}

bool usb_log_device_connected() {
    return tud_cdc_n_connected(LOG_ITF);
}

bool usb_log_port_listening() {
// Sensing DTR doesn't seem to work    return (tud_cdc_n_get_line_state(LOG_ITF) & CDC_CONTROL_LINE_STATE_DTR) == 1;
    cdc_line_coding_t coding;
    tud_cdc_n_get_line_coding(LOG_ITF, &coding);
    return coding.bit_rate != 0; // minicom sets the baud rate on connect.
}

void usb_log_wait_for_connected() {
    while (true) {
        usb_poll();
        if (tud_cdc_n_ready(LOG_ITF) && tud_cdc_n_connected(LOG_ITF)) {
            return;
        }
    }
}

void usb_log_wait_for_knock() {
    while (knock_state != Knocked) {
        usb_poll();
    }
}

void usb_log_write(void *buf, uint32_t size) {
    if (tud_cdc_n_connected(LOG_ITF)) {
        _usb_write(LOG_ITF, buf, size);
    }
    usb_poll();
}


uint32_t usb_log_read(void *buf, uint32_t size) {
    // One does not read from the log interface!
    return 0;
}

void usb_log_flush() {
    if (tud_cdc_n_connected(LOG_ITF)) {
        tud_cdc_n_write_flush(LOG_ITF);
    }
    usb_poll();
}