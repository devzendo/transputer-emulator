//------------------------------------------------------------------------------
//
// File        : sync.h
// Description : Synchronisation primitives (mostly for Pico)
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 08/12/2025
//
// (C) 2005-2025 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _SYNC_H
#define _SYNC_H

#ifdef PICO
#include <pico/sync.h>
#endif


#ifdef PICO
struct CriticalSection  /* BasicResolvable */ {
    CriticalSection() {
        critical_section_init(&m_critsec);
    }

    void lock() {
        critical_section_enter_blocking(&m_critsec);
    }

    void unlock() {
        critical_section_exit(&m_critsec);
    }

    CriticalSection(const CriticalSection&) = delete;

    CriticalSection& operator=(const CriticalSection&) = delete;

private:
    critical_section_t m_critsec{};
};

#endif // PICO

#endif // _SYNC_H