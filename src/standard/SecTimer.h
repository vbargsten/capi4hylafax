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
#ifndef _SECTIMER_H_
#define _SECTIMER_H_

#include "CBTimer.h"

/*===========================================================================*\
\*===========================================================================*/

typedef tHandle thSecTimer;

tBool SecTimer_Create (thSecTimer *pTimer);
void SecTimer_Destroy (thSecTimer *pTimer);

tBool SecTimer_Call   (thSecTimer timer, tUInt secs, tCallBackTimerFunc cbfunc,
                       void *handle1 = 0, void *handle2 = 0);
tBool SecTimer_Cancel  (thSecTimer timer);      // vFalse = cancel failed = timer is signaling or has signaled

/*===========================================================================*\
\*===========================================================================*/
#ifdef SEC_TIMER_USE_MS_TIMER

#include "msTimer.h"

inline tBool SecTimer_Create (thSecTimer *pTimer) {
    return msTimer_Create (pTimer);
}

inline void SecTimer_Destroy (thSecTimer *pTimer) {
    msTimer_Destroy (pTimer);
}

inline tBool SecTimer_Call (thSecTimer timer, tUInt secs, tCallBackTimerFunc cbfunc, void *handle1, void *handle2) {
    // TODO: Ist Typ tUInt vielleicht nicht etwas zu klein??
    return msTimer_Call (timer, secs * 1000, cbfunc, handle1, handle2);
}

inline tBool SecTimer_Cancel (thSecTimer timer) {
    return msTimer_Cancel (timer);
}

#endif
/*===========================================================================*\
\*===========================================================================*/

#endif
