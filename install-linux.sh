#!/bin/bash

PARACHUTEPREFIX=/opt/parachute
BINDIR=$PARACHUTEPREFIX/bin

sudo mkdir -p $PARACHUTEPREFIX
sudo mkdir -p $BINDIR

if [ -f cmake-build-debug/IServer/server/iserver ]; then
	sudo cp cmake-build-debug/IServer/server/iserver $BINDIR
fi
if [ -f cmake-build-debug/Emulator/temulate ]; then
	sudo cp cmake-build-debug/Emulator/temulate $BINDIR
fi


sudo chmod 755 $BINDIR/iserver
sudo chmod 755 $BINDIR/temulate

