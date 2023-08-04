#!/bin/bash
# Once you have done a full dependency-download/compile-dependencies build with
# mvn clean compile -P build
# then this will build just the emulator and IServer.
cmake --build cmake-build-debug --target all -- -j 4
# Use install-xxxx.sh to install on your system.

