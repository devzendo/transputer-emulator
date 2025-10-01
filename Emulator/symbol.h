//------------------------------------------------------------------------------
//
// File        : symbol.h
// Description : Symbol table management
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 21/08/2023
//
// (C) 2005-2025 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _SYMBOL_H
#define _SYMBOL_H

#include <map>

#include "types.h"

#ifdef DESKTOP

class SymbolTable {
	public:
		SymbolTable();
		void addSymbol(std::string name, WORD32 addr);
		bool symbolExists(std::string name);
		WORD32 getSymbolValue(std::string name); // precondition: it's known to exist
		std::string getSymbolName(WORD32 addr); // precondition: it's known to exist
		bool addressExists(WORD32 addr);
		std::string symbolOrEmptyString(WORD32 addr);
		char *possibleSymbol(WORD32 addr); // note: static storage, same address returned each time - copy it!
		std::string possibleSymbolString(WORD32 addr);
		~SymbolTable();
	private:
		map<std::string, WORD32> symbolToAddress;
		map<WORD32, std::string> addressToSymbol;
};

#endif // DESKTOP

#endif // _SYMBOL_H

