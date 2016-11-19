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

#include "Eventing.h"
#include "Protect.h"
#include "dbgLINUX.h"
#include <semaphore.h>
#include <errno.h>

/*===========================================================================*\
\*===========================================================================*/

tBool Event_Create (thEvent *pEvent) {
    dhead ("Event_Create", DEBUG_Eventing);
    dassert (pEvent != 0);
    dwarning (*pEvent == vIllegalHandle);
    dassert (vIllegalHandle == 0);
    sem_t *pSem = new sem_t;
    if (pSem && (sem_init (pSem, 0, 0) == 0)) {
        *pEvent = pSem;
        return vTrue;
    }
    dprintWrn ("sem_init failed with %d. ", errno);
    *pEvent = 0;
    return vFalse;
}


/*===========================================================================*\
\*===========================================================================*/

void Event_Destroy (thEvent *pEvent) {
    dhead ("Event_Destroy", DEBUG_Eventing);
    dassert (pEvent != 0);
    if (*pEvent != vIllegalHandle) {
        if (sem_destroy ((sem_t *)*pEvent) != 0) {
            dprintErr ("sem_destroy failed with %d. ", errno);
        }
        delete (sem_t *)*pEvent;
        *pEvent = vIllegalHandle;
    }
}


/*===========================================================================*\
\*===========================================================================*/

void Event_Set (thEvent event) {
    dhead ("Event_Set", DEBUG_Eventing);
    dassert (event != 0);
    int get;
    sem_getvalue ((sem_t *)event, &get);
    if (get == 0) {
        int fret = sem_post ((sem_t *)event);
        dassert (fret == 0);
    }
}


/*===========================================================================*\
\*===========================================================================*/

void Event_Clear (thEvent event) {
    dhead ("Event_Clear", DEBUG_Eventing);
    dassert (event != 0);
    while (sem_trywait ((sem_t *)event) == 0)
        ;
}


/*===========================================================================*\
\*===========================================================================*/

tBool Event_Wait (thEvent event, tUInt TimeInMSec) {
    dhead ("Event_Wait", DEBUG_Eventing);
    dparams ("%x", TimeInMSec);
    dassert (TimeInMSec == 0);  // No timeout support now !!
    while (sem_wait ((sem_t *)event)) {
        if (errno != EINTR) {
            break;
        }
    }
    return vTrue;
}


/*===========================================================================*\
\*===========================================================================*/

