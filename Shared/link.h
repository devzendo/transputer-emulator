//------------------------------------------------------------------------------
//
// File        : link.h
// Description : Abstract base class for links
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 18/07/2005
//
// (C) 2005-2019 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _LINK_H
#define _LINK_H

#include <exception>
using namespace std;

#include "platformdetection.h"
#include "types.h"

#ifdef PLATFORM_WINDOWS
// For MSVC, shut up about throw (exception)
#pragma warning( disable : 4290 )
#endif

class Link {
	public:
		Link(int linkNo, bool isServer);
		virtual void initialise(void) throw (exception) = 0;
		virtual ~Link(void);
		// TODO may need buffer-centric versions of these, and
		// these are entirely synchronous. to prevent blocking I/O
		// suspending the whole emulator, I should make each link
		// driven by a state machine to introduce some polled
		// asynchrony.
		// For now, just to get data flowing from the Node Server,
		// I'll use the synchronous forms.
		virtual BYTE readByte(void) throw (exception) = 0;
		virtual void writeByte(BYTE b) throw (exception) = 0;
		WORD32 readWord(void) throw (exception);
		void writeWord(WORD32 w) throw (exception);
		virtual void resetLink(void) throw (exception) = 0;
		int getLinkNo(void);
		void setDebug(bool newDebug);
	protected:
		int myLinkNo;
		bool bServer;
		bool bDebug;
};

#endif // _LINK_H

