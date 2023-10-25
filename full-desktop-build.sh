#!/bin/bash
rm -rf cmake-build-debug/ ; mvn clean
mvn compile -P build

