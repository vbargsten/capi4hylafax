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

#ifndef _SEMAFUNC_H_
#define _SEMAFUNC_H_

#include "aTypes.h"

/*===========================================================================*\
\*===========================================================================*/

typedef tHandle thSemaphore;

tBool Semaphore_Create  (thSemaphore *pSema);
void  Semaphore_Destroy (thSemaphore *pSema);

void  Semaphore_Inc     (thSemaphore sema);
tBool Semaphore_Wait    (thSemaphore sema, tUInt TimeInMSec = 0);  // TimeInMSec == 0 => wait infinite
                                                                   // return: vFalse => Timer was executed

/*===========================================================================*\
\*===========================================================================*/

#endif

