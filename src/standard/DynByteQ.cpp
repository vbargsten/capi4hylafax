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

#include "DynByteQ.h"
#include "dbgSTD.h"

/*===========================================================================*\
    CDynamicByteQueue::Put
\*===========================================================================*/

tUInt CDynamicByteQueue::Put (tUByte *dataIn, tUInt length) {
    dhead ("CDynamicByteQueue::Put", DCON_CStandardByteQueue);
    if (FreeSize() < length) {
        Resize ((MaxSize() >= (MAXVAL_tUInt / 2)) ? MAXVAL_tUInt : ((MaxSize() * 2) + 1));
    }
    return CStandardByteQueue::Put (dataIn, length);
}


/*===========================================================================*\
    CDynamicByteQueue::PutInFront
\*===========================================================================*/

tUInt CDynamicByteQueue::PutInFront (tUByte *dataIn, tUInt length) {
    dhead ("CDynamicByteQueue::PutInFront", DCON_CStandardByteQueue);
    if (FreeSize() < length) {
        Resize ((MaxSize() >= (MAXVAL_tUInt / 2)) ? MAXVAL_tUInt : ((MaxSize() * 2) + 1));
    }
    return CStandardByteQueue::PutInFront (dataIn, length);
}


/*===========================================================================*\
\*===========================================================================*/
