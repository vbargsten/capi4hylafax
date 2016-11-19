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

#ifndef _LOOPTIME_H_
#define _LOOPTIME_H_

#include "CBTimer.h"

/*===========================================================================*\
\*===========================================================================*/

typedef tHandle thLoopTimer;

tBool LoopTimer_Create (thLoopTimer *pTimer);
void LoopTimer_Destroy (thLoopTimer *pTimer);

tBool LoopTimer_Start (thLoopTimer timer, tUInt msecs, tCallBackTimerFunc cbfunc,
                       void *handle1 = 0, void *handle2 = 0);
void LoopTimer_Change (thLoopTimer timer, tCallBackTimerFunc cbfunc, void *handle1 = 0, void *handle2 = 0);
void LoopTimer_Stop (thLoopTimer timer);

/*===========================================================================*\
\*===========================================================================*/
#ifdef USE_TIMER_POOL

tUInt LoopTimerPool_Alloc (tUInt count);     // return successfull allocated Timers
void  LoopTimerPool_Free  (tUInt count = 0);

#endif
/*===========================================================================*\
\*===========================================================================*/

#endif

