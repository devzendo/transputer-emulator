# Current Development Activities

* Adding support for the Pi Pico: 
  * Asynchronous link abstraction using GPIO:
    * Lowest level: TxRxPin, represents a pair of abstract pins. GPIOTxRxPin would use Pi Pico
      GPIO pins. Tests would use a CrosswiredTxRxPinPair, which gives a pair of TxRxPins, A and B,
      where setting A's Tx pin enables B's Rx pin. Setting B's Tx enables A's Rx. An AsyncLink
      would take a TxRxPin, and tests would create two AsyncLinks with the two TxRxPins back-to-back.
    * OversampledTxRxPin handles the potentially noisy input and performs majority voting on it to yield
      a cleaner set of bit-long samples to the next layer...
    * Medium level: DataAckSender, state machine that uses the Tx half of a TxRxPin to clock out an
      Ack or Data frame and can be queried for its state. DataAckReceiver, a state machine that
      senses the Rx half of a TxRxPin to clock in any received Ack and/or Data frame.
    * High level: AsyncLink.
  * Need a Pico USB CDC link, supported on the IServer side by a link that works with a TTY. (Done, although
    will require porting for Windows, and probably some refinement of the POSIX TTY code on Linux/macOS).
  * Also need logging to go out on a second USB CDC port (Done).
  * Refactoring all link abstractions to be asynchronous, and reworking the boot/IO code to work with this.
  * Learning PIO, to implement a link that doesn't incur CPU usage.
  * PIO link that works at multiple speeds: 10Mbps for compatibility, max PIO speed, low speed for connection to
    other microcontroller ports that have to bit-bang (Cheap Yellow Display perhaps?)
  * Make use of both ARM cores:
    * Inter-core link: Core 0's Link 1 could be connected to Core 1's Link 0.
    * Links:
      * Core 0:
        * Link 0: USB CDC
        * Link 1: To Core 1 Link 0
        * Link 2: 10MHz original Transputer Async link protocol
        * Link 3: Faster Async link protocol
* Port emulator to the Cheap Yellow Display (ESP32-2432S028), with a Device Server on one core: 
  * Wifi, Bluetooth, Touch screen, bitmap display, SD card reader, RGB LED, Light dependent resistor, Speaker
* Testing with the Transputer Validation Suite: all implemented instructions
  pass; need to assess whether the other instructions testable via the TVS but
  not yet currently implemented are required, and implement them.
* Debugging eForth port.
* Finishing all protocol frames of the IServer (ongoing; led by what eForth and examples need).
* Need to remove use of C++ exceptions? Linux Mint 21 build is warning about it.
* The TVS tests currently run in the emulator but it would be useful to have a server that can work
  with the embedded emulator to send/collect program/input and output.
* Do all TVS programs end with a start instruction? If not, how will the embedded run of all tests
  restart the embedded emulator for the next test?

# Known problems
* The bit-banged GPIO link is a proof of concept, and does not work at a usable data rate.

