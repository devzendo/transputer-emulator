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

#ifdef PICO
#include <stdio.h> // Pico USB Serial STDIO
#include "pico/stdlib.h"
#endif

using namespace std;

int main() {
    // Initialise USB Serial STDIO...
    bool ok = stdio_init_all();
    printf("Hello from picolinkreflect.uf2 %d\r\n", ok);
    // LED
    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);
}
