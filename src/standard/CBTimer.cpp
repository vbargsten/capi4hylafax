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

#include "CBTimer.h"
#include "dbgSTD.h"

/*===========================================================================*\
\*===========================================================================*/
#ifndef USE_TIMER_POOL

tBool CBasicTimerSupport::Create (tHandle *pTimer, tBasicTimerInitFunc func) {
    dhead ("CBasicTimerSupport::Create", DCON_CBasicTimerSupport);
    dassert (func != 0);
    *pTimer = (tHandle)func();
    dwarning (*pTimer != 0);
    RETURN ('x', (*pTimer) ? vTrue : vFalse);
}


void CBasicTimerSupport::Destroy (tHandle *pTimer) {
    dhead ("CBasicTimerSupport::Destroy", DCON_CBasicTimerSupport);
    if (*pTimer) {
        delete (CBasicTimerSupport *)*pTimer;
        *pTimer = 0;
    }
}


/*===========================================================================*\
\*===========================================================================*/
# else // #ifndef USE_TIMER_POOL

struct CBTimerPool_Type {
    CBasicTimerSupport *first;
    tUInt               count;
    Protect_Define     (hProtect);
};

static CBTimerPool_Type CBTimerPool = { 0 };


/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

tBool CBasicTimerSupport::Create (tHandle *pTimer, tBasicTimerInitFunc) {
    dhead ("CBasicTimer(Pool)Support::Create", DCON_CBasicTimerSupport);
    dassert (CBTimerPool.count > 0);
    dassert (pTimer != 0);
    Protect_BeginWrite (CBTimerPool.hProtect);
    dwarning (CBTimerPool.first != 0);          // Pool has a to small dimension!!
    if (CBTimerPool.first) {
        *pTimer           = (tHandle)CBTimerPool.first;
        CBTimerPool.first = CBTimerPool.first->m_next;
        Protect_EndWrite (CBTimerPool.hProtect);
        ((CBasicTimerSupport *)*pTimer)->m_next = 0;
        RETURN ('x', vTrue);
    }
    Protect_EndWrite (CBTimerPool.hProtect);
    *pTimer = 0;
    RETURN ('x', vFalse);
}

void CBasicTimerSupport::Destroy (tHandle *pTimer) {
    dhead ("CBasicTimer(Pool)Support::Destroy", DCON_CBasicTimerSupport);
    dassert (CBTimerPool.count > 0);
    dassert (pTimer != 0);
    dassert (*pTimer != 0);
    Protect_BeginWrite (CBTimerPool.hProtect);
    ((CBasicTimerSupport *)*pTimer)->m_next = CBTimerPool.first;
    CBTimerPool.first                       = (CBasicTimerSupport *)*pTimer;
    Protect_EndWrite (CBTimerPool.hProtect);
    *pTimer = 0;
}


/*===========================================================================*\
\*===========================================================================*/

tUInt BasicTimerPool_Alloc (tUInt count, tBasicTimerInitFunc func) {
    dhead ("BasicTimerPool_Alloc", DCON_CBasicTimerSupport);
    dassert (count > 0);
    dassert (func != 0);
    if (CBTimerPool.count == 0) {
        dprint ("-init- ");
        if (Protect_Create (&CBTimerPool.hProtect) == vFalse) {
            dassert (0);
            return ('x', 0);
        }
    }

    tHandle h;
    tUInt   i = 0;
    for (; i < count; i++) {
        h = (tHandle)func();
        if (!h) {
            break;
        }
        CBTimerPool.count++;
        CBasicTimerSupport::Destroy (&h);
    }
    if (!CBTimerPool.count) {
        dwarning (0);
        // no timer is allocated!
        Protect_Destroy (&CBTimerPool.hProtect);
    }
    dwarning (count == i);
    RETURN ('x', i);
}


/*===========================================================================*\
\*===========================================================================*/

void BasicTimerPool_Free (tUInt count) {
    dhead ("BasicTimerPool_Free", DCON_CBasicTimerSupport);
    dparams ("%x", count);
    dassert (CBTimerPool.count > 1);
    tHandle h;
    if (count && (count <= CBTimerPool.count)) {
        for (; count > 0; count--) {
            if (CBasicTimerSupport::Create (&h, 0) == vFalse) {
                dassert (0);
                break;
            }
            dassert (h != 0);
            dassert (CBTimerPool.count > 0);
            CBTimerPool.count--;
            delete (CBasicTimerSupport *)h;
        }
    } else {
        while (CBTimerPool.count && (CBasicTimerSupport::Create (&h, 0) == vTrue)) {
            dassert (CBTimerPool.count > 0);
            CBTimerPool.count--;
            dassert (h != 0);
            delete (CBasicTimerSupport *)h;
        }
        dassert (CBTimerPool.count == 0);
        dassert (CBTimerPool.first == 0);
        CBTimerPool.count = 0;
        Protect_Destroy (&CBTimerPool.hProtect);
    }
}


/*===========================================================================*\
\*===========================================================================*/
#endif // USE_TIMER_POOL
