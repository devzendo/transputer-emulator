#!/bin/bash
rm -rf cmake-build-debug/ ; mvn clean
export PICO_SDK_PATH=~/Documents/pico-sdk-experiments/pico-sdk
mvn -DCROSS=PICO compile -P build

