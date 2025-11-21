#!/bin/bash
rm -rf cmake-build-debug
rm -rf cmake-build-release
mvn clean
mvn compile -P build

