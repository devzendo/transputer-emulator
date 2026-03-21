#!/bin/bash

PARACHUTEPREFIX=/opt/parachute
BINDIR=$PARACHUTEPREFIX/bin

sudo mkdir -p $PARACHUTEPREFIX
sudo mkdir -p $BINDIR

if [ -f cmake-build-release/IServer/server/iserver ]; then
	sudo cp cmake-build-release/IServer/server/iserver $BINDIR
	# idiocy for macos sometimes needed?!
	sudo codesign --sign - --force --preserve-metadata=entitlements,requirements,flags,runtime $BINDIR/iserver
fi
if [ -f cmake-build-release/Emulator/temulate ]; then
	sudo cp cmake-build-release/Emulator/temulate $BINDIR
fi


sudo chmod 755 $BINDIR/iserver
sudo chmod 755 $BINDIR/temulate

