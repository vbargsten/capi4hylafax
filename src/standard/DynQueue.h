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

#ifndef _DYNQUEUE_H_
#define _DYNQUEUE_H_

#include "StdQueue.h"

/*===========================================================================*\
\*===========================================================================*/

class CDynamicQueue : public CStandardQueue {
public:
    CDynamicQueue (tUInt StartCountOfItems, tUInt SizeOfItem) :
                CStandardQueue (StartCountOfItems, SizeOfItem) {}

    tBool Put (void *dataIn);
    tBool PutInFront (void *dataIn);
};

/*===========================================================================*\
\*===========================================================================*/

#endif
