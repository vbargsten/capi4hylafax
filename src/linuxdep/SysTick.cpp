/*---------------------------------------------------------------------------*\

    Copyright (C) 2000 AVM GmbH. All rights reserved.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY, without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, and WITHOUT
    ANY LIABILITY FOR ANY DAMAGES arising out of or in connection
    with the use or performance of this software. See the
    GNU General Public License for further details.

\*---------------------------------------------------------------------------*/

#include <sys/time.h>
#include <signal.h>
#include <assert.h>
#include "SysTick.h"

static CSystemTicker *ticker = 0;

/*---------------------------------------------------------------------------*\
 * Run-time system's signal handler
\*---------------------------------------------------------------------------*/

static void tick_handler (int /*sig*/) {
    if (ticker) {
        ticker->IncrementTicks ();
        ticker->NotifyTimers ();
    }
} /* tick_handler */

/*---------------------------------------------------------------------------*\
 * CSystemTicker
\*---------------------------------------------------------------------------*/

int CSystemTicker::alife = 0;

CSystemTicker::CSystemTicker (void) :
    first (0),
    list (0),
    ticks (0),
    handler (0)
{
    if (1 == ++alife) {
        struct itimerval interval;

        list = new CTimerList ();
        handler = signal (SIGALRM, &tick_handler);
        interval.it_interval.tv_sec  = 1;
        interval.it_interval.tv_usec = 0;
        interval.it_value.tv_sec     = 1;
        interval.it_value.tv_usec    = 0;
        (void) setitimer (ITIMER_REAL, &interval, 0);
        first = this;
    } else {
        first = GetSystemTicker ();
    }
} // CSystemTicker::CSystemTicker

CSystemTicker::~CSystemTicker (void) {
    if (0 == --alife) {
        struct itimerval interval;

        interval.it_interval.tv_sec  = 0;
        interval.it_interval.tv_usec = 0;
        interval.it_value.tv_sec     = 0;
        interval.it_value.tv_usec    = 0;
        (void) setitimer (ITIMER_REAL, &interval, 0);
        (void) signal (SIGALRM, handler);
        handler = 0;
        delete list;
    }
    first = 0;
} // CSystemTicker::~CSystemTicker

void CSystemTicker::AddTimer (tHandle timer, unsigned interval) {
    // Something in the setitimer code limits the maximum duration
    // for these timers. I have not yet determined the exact maximum,
    // but 30 minutes works, an hour does not. (The corresponding
    // timer will not "fire"...)
    //
    assert (interval < 1801);
    list->Add (timer, ticks + interval);
} // CSystemTicker::AddTimer

tBool CSystemTicker::RemoveTimer (tHandle timer) {
    return list->Remove (timer);
} // CSystemTicker::RemoveTimer

void CSystemTicker::NotifyTimers (void) {
    list->Notify (ticks);
} // CSystemTicker::NotifyTimers

/*---------------------------------------------------------------------------*\
 * Global access to the global system ticker object
\*---------------------------------------------------------------------------*/

CSystemTicker * GetSystemTicker (void) {
    return ticker ? ticker : ticker = new CSystemTicker ();
} // SetSystemTicker

