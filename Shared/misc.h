//------------------------------------------------------------------------------
//
// File        : misc.h
// Description : Miscellaneous functions and stuff
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 15/07/2005
//
// (C) 2005-2024 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _MISC_H
#define _MISC_H

#include <string>
#include "platformdetection.h"

#if defined(PLATFORM_WINDOWS)
extern std::string GetLastErrorStdStr();
#endif

// Throw a runtime error with a prefix string, followed by the last error text (from strerror or GetLastErrorStdStr())
extern void throwLastError(const std::string &prefix /* "It failed with: " */);

// Obtain the text of the last error (from strerror or GetLastErrorStdStr())
extern std::string getLastError();


// Thanks to Torsten, from https://stackoverflow.com/questions/12261915/how-to-throw-stdexceptions-with-variable-messages
#include <stdexcept>
#include <sstream>

class Formatter
{
public:
    Formatter() = default;
    ~Formatter() = default;

    template <typename Type>
    Formatter & operator << (const Type & value) {
        stream_ << value;
        return *this;
    }

    std::string str() const {
        return stream_.str();
    }

    operator std::string () const {
        return stream_.str();
    }

    enum ConvertToString {
        to_str
    };
    std::string operator >> (ConvertToString) {
        return stream_.str();
    }

private:
    std::stringstream stream_;

    Formatter(const Formatter &);
    Formatter & operator = (Formatter &);
};


// Strip all occurrences of a character from the end of a string.
// Return the original string if it's empty or needs no stripping, otherwise, returns a stripped version.
extern std::string stripTrailing(char toStrip, const std::string &from);

// String all occurrences of a character from the start of a string
// Return the original string if it's empty or needs no stripping, otherwise, returns a stripped version.
extern std::string stripLeading(char toStrip, const std::string &from);

#endif // _MISC_H

