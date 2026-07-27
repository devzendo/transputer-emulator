/* -----------------------------------------------------------------------------
//
// File        : bin2boot.c
// Description : prepend a bootloader to an assembled binary program.
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 27/07/2026
//
// (C) 2005-2026 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
// The Small-C compiler emits position-independent code that could be loaded into an emulator at MemStart and run, but
// it does not contain any initialisation code. Directly patching into memory is an atypical way to run a program on a
// Transputer: you either send it down a link, or boot from ROM. Neither can be done directly with Small-C output. It
// can't be sent down a link since it has no link boot length byte, and may be greater than 255 bytes, requiring a boot
// loader. Nor does it have any Transputer initialisation code. It can't be run from ROM since it has no jump sequence
// at the end.
//
// Hence this program.
//
// Take an input binary file assembled by tasm_modern, and prepend a bootloader and initialisation code.
// The output binary can be sent down a link. It contains:
// The length byte giving the length of the bootloader. This code will be read by the Transputer during its boot stage.
// This code will initialise the transputer, then request a further word down the link. The contents of this word are
// set by this bin2boot program, based on the size of the input assembled binary that the program is invoked with.
// Now that the bootloader knows the length of the assembled binary, that many bytes are read into memory just after
// the bootloader. The start of this code is then executed by the bootloader.
//
// The bootloader code is taken from the transputer-macro-assembler project, at
// tma-includes/src/main/resources/include/tmasm/boot/bootstrap.asm, specifically by looking at the bytes generated
// in a listing of a program that uses it. This includes the binary output.
// (IServer/client-examples/hello-world-secondary-server/hello.lst)
// Take care at the end of it, as there's an alignment that needs preserving.
//--------------------------------------------------------------------------- */

#include <cstdio>
#include <iostream>
#include <iomanip>

#include "types.h"
#include "misc.h"

// Bootloader code
const BYTE8 bootloader[] = {
    0xB0, // boot 1 length
    0x22, 0xB8,
    0xD6,
    0xD5,
    0xD4,
    0x74,
    0x60, 0x5C,
    0xD3,
    0x24, 0xF2,
    0x21, 0xF8,
    0x24, 0xF2,
    0x21, 0xFC,
    0x24, 0xF2,
    0x67, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x22, 0x44,
    0xE0,
    0x24, 0xF2,
    0x67, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x22, 0x48,
    0xE0,
    0x40,
    0x25, 0xF4,
    0x22, 0xF9,
    0x25, 0xF7,
    0x29, 0xFC,
    0x24, 0xF2,
    0x67, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x22, 0x40,
    0xE0,
    0x24, 0xF2,
    0x67, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x21, 0x4C,
    0xE0,
    0x24, 0xF2,
    0x67, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x21, 0x48,
    0xE0,
    0x24, 0xF2,
    0x67, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x21, 0x44,
    0xE0,
    0x24, 0xF2,
    0x67, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x21, 0x40,
    0xE0,
    0x24, 0xF2,
    0x67, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x4C,
    0xE0,
    0x24, 0xF2,
    0x67, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x48,
    0xE0,
    0x24, 0xF2,
    0x67, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x44,
    0xE0,
    0x24, 0xF2,
    0x67, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x40,
    0xE0,

    0x67, 0x20, 0x20, 0x20,
    0x20, 0x21, 0x22, 0x40,
    0x74,
    0x4D,
    0x21, 0xFB,
    0x30,
    0xF7,
    0x67, 0x20, 0x20, 0x20,
    0x20, 0x21, 0x22, 0x40,
    0xF6,   // 80000119
    // ALIGN 4
    0x00,   // 8000011A
    0x00,   // 8000011B
            // 8000011C
    // And here at 8000011C we write the program size word in little endian.
};

void printHex32(WORD32 value) {
    std::ios::fmtflags f(std::cout.flags());   // save state
    std::cout << "0x"
               << std::hex << std::uppercase
               << std::setfill('0') << std::setw(8)
               << value;
    std::cout.flags(f);                        // restore state
}

// Precondition: file exists.
unsigned long fsize(const char* file) {
    FILE *f = fopen(file, "r");
    fseek(f, 0, SEEK_END);
    auto len = static_cast<unsigned long>(ftell(f));
    fclose(f);
    return len;
}

void toLittleEndian(WORD32 value, BYTE8 *littleEndian) {
    littleEndian[0] = static_cast<BYTE8>(value & 0x000000ff);
    littleEndian[1] = static_cast<BYTE8> ((value & 0x0000ff00) >> 8);
    littleEndian[2] = static_cast<BYTE8> ((value & 0x00ff0000) >> 16);
    littleEndian[3] = static_cast<BYTE8> ((value & 0xff000000) >> 24);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: bin2boot <input filename> <output filename>" << std::endl;
        exit(1);
    }
    char *input = argv[1];
    char *output = argv[2];
    FILE *in = fopen(input, "r");
    if (in == nullptr) {
        std::cerr << "Can't open input file " << input << ": " << getLastError() << std::endl;
        exit(1);
    }
    FILE *out = fopen(output, "w");
    if (out == nullptr) {
        std::cerr << "Can't create output file " << output << ": " << getLastError() << std::endl;
        fclose(in);
        exit(1);
    }
    int retval = 0;
    std::cout << "Reading from "<< input << std::endl;
    std::cout << "Writing boot loader to " << output << std::endl;
    if (fwrite(&bootloader, 1, sizeof(bootloader), out) != sizeof(bootloader)) {
        std::cerr << "Can't write boot loader to output file " << output << ": " << getLastError() << std::endl;
        retval = 1;
    } else {
        WORD32 size = fsize(input);
        std::cout << "Writing program length of " << size << " bytes (";
        printHex32(size);
        std::cout << ") to " << output << std::endl;
        BYTE8 littleEndianSize[4];
        toLittleEndian(size, littleEndianSize);
        if (fwrite(&littleEndianSize, 1, 4, out) != 4) {
            std::cerr << "Can't write program length to output file " << output << ": " << getLastError() << std::endl;
            retval = 1;
        } else {
            std::cout << "Appending program from " << input << " to " << output << std::endl;
            BYTE8 buffer[128];
            while (true) {
                size_t nread = fread(buffer, 1, sizeof(buffer), in);
                if (nread == 0) { // EOF
                    std::cout << "Finished" << std::endl;
                    break;
                }
                size_t nwritten = fwrite(buffer, 1, nread, out);
                if (nwritten != nread) {
                    std::cerr << "Can't write program from " << input << ": " << getLastError() << std::endl;
                    retval = 1;
                    break;
                }
            }
        }
    }
    fclose(in);
    fclose(out);
    exit(retval);
}
