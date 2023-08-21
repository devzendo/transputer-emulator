//------------------------------------------------------------------------------
//
// File        : symbol.h
// Description : Symbol table management
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 21/08/2023
//
// (C) 2005-2023 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _SYMBOL_H
#define _SYMBOL_H

#include <map>

#include "types.h"

class SymbolTable {
	public:
		SymbolTable();
		void addSymbol(std::string name, WORD32 addr);
		bool symbolExists(std::string name);
		bool addressExists(WORD32 addr);
		std::string symbolOrEmptyString(WORD32 addr);
		char *possibleSymbol(WORD32 addr); // note: static storage, same address returned each time - copy it!
		~SymbolTable();
	private:
		map<std::string, WORD32> symbolToAddress;
		map<WORD32, std::string> addressToSymbol;
};

#endif // _SYMBOL_H

