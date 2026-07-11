//------------------------------------------------------------------------------
//
// File        : sync.h
// Description : Synchronisation primitives (mostly for Pico)
// License     : Apache License v2.0 - see LICENSE.txt for more details
// Created     : 08/12/2025
//
// (C) 2005-2026 Matt J. Gumbley
// matt.gumbley@devzendo.org
// http://devzendo.github.io/parachute
//
//------------------------------------------------------------------------------

#ifndef _SYNC_H
#define _SYNC_H

#include <mutex> // For std::lock_guard and BasicLockable

#ifdef PICO
#include <pico/sync.h>
#endif

#ifdef PICO
#include <pico/sync.h>
// uint is picked up from pico/types.h
#else
typedef unsigned int uint; // which is what pico/types.h defines it as.
#endif

/*
 * Access to internals is protected by this MUTEX definition - a std::lock_guard
 * and an appropriate lock for the platform - std::mutex on desktop, and a critical_section_t
 * on PICO, wrapped in this BasicResolvable.
 * Any class you want to have a MUTEX must define an m_mutex (desktop) or m_criticalsection (PICO).
 * Then use the MUTEX macro which will acquire the guard, letting it go out of scope to release.
 */

#ifdef DESKTOP
#define MUTEX     std::lock_guard<std::mutex> guard(m_mutex);
#endif

#ifdef PICO
#define MUTEX     std::lock_guard<CriticalSection> guard(m_criticalsection);
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