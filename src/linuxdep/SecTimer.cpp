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

#include "SecTimer.h"
#include "SysTick.h"
#include "dbgLINUX.h"

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

static CBasicTimerSupport *CTimerSlot_AllocFunc (void) {
    return (GetSystemTicker()) ?  new CTimerSlot : 0;
}


/*===========================================================================*\
\*===========================================================================*/

tBool SecTimer_Create (thSecTimer *pTimer) {
    dhead ("SecTimer_Create", DCON_SecTimer);
    dassert (pTimer != 0);
    return CTimerSlot::Create (pTimer, CTimerSlot_AllocFunc);
}

void SecTimer_Destroy (thSecTimer *pTimer) {
    dhead ("SecTimer_Destroy", DCON_SecTimer);
    dassert (pTimer != 0);
    if (*pTimer != vIllegalHandle) {
        SecTimer_Cancel (*pTimer);
        CTimerSlot::Destroy (pTimer);
    }
}

tBool SecTimer_Call (thSecTimer timer, tUInt secs, tCallBackTimerFunc cbfunc, void *handle1, void *handle2) {
    dhead ("SecTimer_Call", DCON_SecTimer);
    dparams ("%x,%x,%x,%x,%x", timer, secs, cbfunc, handle1, handle2);
    dassert (timer != vIllegalHandle);
    dassert (secs > 0);
    dassert (cbfunc != 0);
    SecTimer_Cancel (timer);
    ((CTimerSlot *)timer)->SetCallBack (cbfunc);
    ((CTimerSlot *)timer)->SetHandles (handle1, handle2);
    GetSystemTicker()->AddTimer (timer, secs);
    return vTrue;
}

tBool SecTimer_Cancel (thSecTimer timer) {
    dhead ("SecTimer_Cancel", DCON_SecTimer);
    dassert (timer != vIllegalHandle);
    return GetSystemTicker()->RemoveTimer (timer);
}


/*===========================================================================*\
\*===========================================================================*/
