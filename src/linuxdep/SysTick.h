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

#ifndef __systick_h__
#define __systick_h__

#include "TimerList.h"

class CSystemTicker {
public:
    CSystemTicker (void);
    virtual ~CSystemTicker (void);

    void AddTimer (tHandle timer, unsigned interval);
    tBool RemoveTimer (tHandle timer);

    void IncrementTicks (void);
    void NotifyTimers (void);

private:
    static int  alife;

    CSystemTicker *first;
    CTimerList    *list;
    unsigned       ticks;
    void         (*handler) (int);
};

inline void CSystemTicker::IncrementTicks (void) {
    ticks++;
}

CSystemTicker *GetSystemTicker (void);

#endif

