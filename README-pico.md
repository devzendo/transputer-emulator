# transputer-emulator for Raspberry Pi Pico

## What is this?

This archive contains a temulate.uf2 file for the Transputer emulator, and associated map/symbol files.

This emulator provides:
* A single-core emulated "integer T805" (later releases will make use of the second core of the Pi Pico, 
and use the Pico FP library to implement more of the T805's floating point processor).
* Two USB CDC ports: 
  * Link 0
  * A diagnostic/log port where debug logs will appear.
* Link 1 is a GPIO bit-banged (i.e. slow) link implementation for connection to other picos. It is present on Pico pins
3 (GND), 4 (TX) and 5 (RX).
* 200KB of RAM available for Transputer code.

There is also linkadapter.uf2 that adapts the GPIO link to USB CDC, so that a host system can connect to future
emulator variants that only provide GPIO links - it's a bit like the C012 link adapter chip, but using USB CDC rather
than parallel connection to a host.

These files are to be copied to a Raspberry Pi Pico by holding down the BOOT button, connecting the board to a desktop
system, then copying the .uf2 file to the Pico's boot drive ('RPI-RP2'). After copying the board will reset (you may get
an OS warning saying that you should unmount the drive cleanly - this can be ignored in this case).

## Example using 'Hello World' and the IServer, on Windows

In addition to the Pico version of the emulator installed on your board, you'll need the desktop version of the
parachute system installed, with its 'bin' directory on your PATH. See the installation instructions in the 
`README-FIRST-parachute.md` file.

Run Device Manager, connect your Pi Pico to your PC via USB cable, and look under 'Ports COM/LPT' to find which ports 
are provided by the emulator on the Pico. The first will be Link 0; the second will be the diagnostics port. For
example, say Link 0 is on COM15:

Run the IServer, connecting it to the emulator on COM15, and sending the 'Hello World' binary to the emulator, then,
entering IServer protocol mode:

`iserver -T0COM15: C:\parachute\examples\hello2\hello2.bin`

That is `-T` followed immediately by the digit zero (for Link 0), then immediately followed by `COM15:` including the
colon.

Running this will send the `hello2.bin` file over the link, which the emulator will then execute. It sends a PUTS
request containing the text, then terminates the emulator.

The IServer should then show:
```
hello world
(still does not return)
Ctrl-C <<- you'll have to interrupt it.
```
