#!/bin/bash
tmasm --debug --level --classes -c -b hello.bin -l hello.lst hello.asm | tee hello.out
xxd < hello.bin > hello.hex
