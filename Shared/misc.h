//------------------------------------------------------------------------------
//
// File        : misc.h
// Description : Miscellaneous functions and stuff
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 15/07/2005
//
// (C) 2005-2025 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _MISC_H
#define _MISC_H

#include <string>
#include <mutex>
#include <condition_variable>
#include "platformdetection.h"
#include "types.h"

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

extern const char *byte_to_binary(BYTE8 x);
extern const char *word_to_binary(WORD16 x);

class CountDownLatch {
public:
    explicit CountDownLatch(unsigned int count) : m_count(count) {}

    void await() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(lock, [this]() { return m_count == 0; });
    }

    void countDown() {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_count > 0) {
            --m_count;
            if (m_count == 0) {
                m_cv.notify_all();
            }
        }
    }

    unsigned int getCount() const {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_count;
    }

private:
    mutable std::mutex m_mutex;
    std::condition_variable m_cv;
    unsigned int m_count;
};

#endif // _MISC_H

