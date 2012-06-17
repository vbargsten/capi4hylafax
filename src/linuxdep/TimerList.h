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

#ifndef __timerlist_h__
#define __timerlist_h__

#include "CBTimer.h"

class CTimerSlot : public CBasicTimerSupport {
public:
    CTimerSlot (void);
    void SetTarget (unsigned target);

private:
    CTimerSlot *link;
    unsigned    target;

    friend class CTimerList;
};

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

class CTimerList {
public:
    CTimerList (void);
    ~CTimerList (void);

    void Add (tHandle timer, unsigned time);
    tBool Remove (tHandle timer);
    void Clear (void);
    void Notify (unsigned time);
    unsigned Card (void);

private:
    CTimerSlot *list;
    unsigned    count;

    CTimerSlot *internal_Remove (CTimerSlot * s1, CTimerSlot * s2);
};

inline unsigned CTimerList::Card (void) {
    return count;
}

#endif
