//------------------------------------------------------------------------------
//
// File        : link.h
// Description : Abstract base class for links
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 18/07/2005
// Revision    : $Revision $
//
// (C) 2005-2018 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _LINK_H
#define _LINK_H

#include <exception>
using namespace std;

#include "types.h"

class Link {
	public:
		Link(int linkNo, bool isServer);
		virtual void initialise(void) throw (exception);
		virtual ~Link(void);
		// TODO may need buffer-centric versions of these, and
		// these are entirely synchronous. to prevent blocking I/O
		// suspending the whole emulator, I should make each link
		// driven by a state machine to indtroduce some polled 
		// asynchrony.
		// For now, just to get data flowing from the Node Server,
		// Ill use the synchronous forms.
		virtual BYTE readByte(void) throw (exception);
		virtual void writeByte(BYTE b) throw (exception);
		WORD32 readWord(void) throw (exception);
		void writeWord(WORD32 w) throw (exception);
		virtual void resetLink(void) throw (exception);
		int getLinkNo(void);
		void setDebug(bool newDebug);
	protected:
		int myLinkNo;
		bool bServer;
		bool bDebug;
};

#endif // _LINK_H

