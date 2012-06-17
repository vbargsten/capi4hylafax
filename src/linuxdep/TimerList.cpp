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

#include "TimerList.h"
#include "dbgLINUX.h"

/*---------------------------------------------------------------------------*\
 * CTimerSlot
\*---------------------------------------------------------------------------*/

CTimerSlot::CTimerSlot (void) :
    link (0),
    target (0)
{
    // NOP
} // CTimerSlot::CTimerSlot

void CTimerSlot::SetTarget (unsigned taRget) {
    target = taRget;
} // CTimerSlot::CTimerSlot


/*---------------------------------------------------------------------------*\
 * CTimerList
\*---------------------------------------------------------------------------*/

CTimerList::CTimerList (void) :
    list (0),
    count (0)
{
    // NOP
} // CTimerList::CTimerList

CTimerList::~CTimerList (void) {
    Clear ();
} // CTimerList::~CTimerList

void CTimerList::Add (tHandle timer, unsigned time) {
    dhead ("CTimerList::Add", DCON_SecTimer);
    dparams ("%x,%x", timer, time);
    CTimerSlot * tmp1 = list;
    CTimerSlot * tmp2 = 0;
    CTimerSlot * new1 = (CTimerSlot *)timer;
    new1->SetTarget (time);

    while (tmp1 && (tmp1->target < time)) {
        tmp2 = tmp1;
        tmp1 = tmp1->link;
    }
    new1->link = tmp1;
    if (tmp2) {
        tmp2->link = new1;
    } else {
        list = new1;
    }
    ++count;
} // CTimerList::Add

tBool CTimerList::Remove (tHandle timer) {
    dhead ("CTimerList::Remove", DCON_SecTimer);
    dparams ("%x", timer);
    CTimerSlot * tmp1 = list;
    CTimerSlot * tmp2 = 0;

    while (tmp1) {
        if (timer == (tHandle)tmp1) {
            (void) internal_Remove (tmp1, tmp2);
            return vTrue;
        }
        tmp2 = tmp1;
        tmp1 = tmp1->link;
    }
    //dwarning (0); // timer not found in list!
    return vFalse;
} // CTimerList::Remove

void CTimerList::Clear (void) {
    dhead ("CTimerList::Clear", DCON_SecTimer);
    CTimerSlot * temp = list;
    CTimerSlot * next;

    while (temp) {
        next = temp->link;
        temp = next;
    }
    count = 0;
    list = 0;
} // CTimerList::Clear

void CTimerList::Notify (unsigned time) {
    dhead ("CTimerList::Notify", DCON_SecTimer);
    dparams ("%x", time);
    CTimerSlot *tmp1 = list;
    CTimerSlot *tmp2 = 0;
    CTimerSlot *tmp3;

    while (tmp1) {
        if (tmp1->target <= time) {
            dprint ("Slot found! ");
            tmp3 = internal_Remove (tmp1, tmp2);
            tmp1->DoCallBack();
            tmp1 = tmp3;
        } else {
            tmp2 = tmp1;
            tmp1 = tmp2->link;
        }
    }
} // CTimerList::Notify

CTimerSlot * CTimerList::internal_Remove (CTimerSlot * s1, CTimerSlot * s2) {
    dhead ("CTimerList::internal_Remove", DCON_SecTimer);
    dparams ("%x,%x", s1, s2);
    CTimerSlot * temp = 0;

    if (s1) {
        temp = s1->link;
        if (s2) {
            s2->link = s1->link;
        } else {
            list = s1->link;
        }
        --count;
    }
    return temp;
} // CTimerList::internal_Remove
