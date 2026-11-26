#!/bin/bash
/opt/parachute/bin/tmasm -c -b scratch.bin -l scratch.lst scratch.asm
python lst2cpp.py < scratch.lst > scratch.h
