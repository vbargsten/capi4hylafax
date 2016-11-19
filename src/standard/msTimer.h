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

#ifndef _MSTIMER_H_
#define _MSTIMER_H_

#include "CBTimer.h"

/*===========================================================================*\
\*===========================================================================*/

typedef tHandle thmsTimer;

tBool msTimer_Create (thmsTimer *pTimer);
void msTimer_Destroy (thmsTimer *pTimer);

tBool msTimer_Call   (thmsTimer timer, tUInt msecs, tCallBackTimerFunc cbfunc,
                      void *handle1 = 0, void *handle2 = 0);
tBool msTimer_Cancel  (thmsTimer timer);    // vFalse = cancel failed = timer is signaling or has signaled

/*===========================================================================*\
\*===========================================================================*/
#ifdef USE_TIMER_POOL

tUInt msTimerPool_Alloc (tUInt count);      // return successfull allocated Timers
void  msTimerPool_Free  (tUInt count = 0);

#endif
/*===========================================================================*\
\*===========================================================================*/

#endif
