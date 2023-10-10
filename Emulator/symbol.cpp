//------------------------------------------------------------------------------
//
// File        : symbol.cpp
// Description : Symbol table manipulation
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 21/08/2023
//
// (C) 2005-2023 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#include <cstdlib>
#include <cctype>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <system_error>
#include <sys/stat.h>
#include <errno.h>
using namespace std;

#ifdef DESKTOP

#include "types.h"
#include "symbol.h"
#include "log.h"

SymbolTable::SymbolTable() {
	logDebug("SymbolTable CTOR");
	symbolToAddress.clear();
	addressToSymbol.clear();
}

void SymbolTable::addSymbol(const std::string name, const WORD32 address) {
//	logDebugF("Symbol added: %s=%08X", name.c_str(), address);
	symbolToAddress[name] = address;
	addressToSymbol[address] = name;
}

bool SymbolTable::symbolExists(std::string name) {
	return symbolToAddress.count(name) == 1;
}

// precondition: it's known to exist
WORD32 SymbolTable::getSymbolValue(std::string name) {
    return symbolToAddress[name];
}

// precondition: it's known to exist
std::string SymbolTable::getSymbolName(WORD32 addr) {
    return addressToSymbol[addr];
}

bool SymbolTable::addressExists(WORD32 address) {
	return addressToSymbol.count(address) == 1;
}

std::string SymbolTable::symbolOrEmptyString(WORD32 address) {
	if (addressToSymbol.count(address) == 1) {
		return addressToSymbol[address];
	} else {
		return "         ";
	}
}

char *SymbolTable::possibleSymbol(WORD32 address) {
	static char buf[255];
	buf[0] = '\0';
	if (addressToSymbol.count(address) == 1) {
		buf[0] = '[';
		strcpy(buf + 1, addressToSymbol[address].c_str());
		strcat(buf, "]");
	}
	return buf;
}

std::string SymbolTable::possibleSymbolString(WORD32 address) {
	std::string buf;
	if (addressToSymbol.count(address) == 1) {
		buf += '[';
		buf += addressToSymbol[address];
		buf += ']';
	}
	return buf;
}



SymbolTable::~SymbolTable() {
	logDebugF("SymbolTable DTOR - this is 0x%lx", this);
}

#endif // DESKTOP


