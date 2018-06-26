//------------------------------------------------------------------------------
//
// File        : linkfactory.h
// Description : Factory for creating derived classes of Link
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 19/07/2005
// Revision    : $Revision $
//
// (C) 2005-2018 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _LINKFACTORY_H
#define _LINKFACTORY_H

const int LinkType_FIFO = 0;
const int LinkType_Socket = 1;
const int LinkType_SharedMemory = 2;

class LinkFactory {
	public:
		LinkFactory(bool isServer, bool isDebug);
		bool processCommandLine(int argc, char *argv[]);
		Link *createLink(int linkNo);
	private:
		int myLinkTypes[4];
		bool bServer;
		bool bDebug;
};

#endif // _LINKFACTORY_H

