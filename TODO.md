# Current Development Activities

* Finishing all protocol frames of the IServer (ongoing; led by what eForth and examples need).
* Testing with the Transputer Validation Suite: all implemented instructions
  pass; need to assess whether the other instructions testable via the TVS but
  not yet currently implemented are required, and implement them.
* Debugging eForth port.
* Need to remove use of C++ exceptions? Linux Mint 21 build is warning about it.
* Adding support for the Pi Pico: 
  * Need a Pico USB Serial link, supported on the IServer side by a FIFO that works with a serial device.
  * Logging needs to be recorded to a memory area, that can be probed by an analyser when the emulator
    is in boot mode. 
  * Use the Maker Pi Pico's NeoRGB LEDs as a boot/run/error state indicator?
* The TVS tests currently run in the emulator but it would be useful to have a server that can work
  with the embedded emulator to send/collect program/input and output.
* Do all TVS programs end with a start instruction? If not, how will the embedded run of all tests
  restart the embedded emulator for the next test?

# Known problems

* None so far.
