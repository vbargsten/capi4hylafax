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

#include "StdQueue.h"
#include "dbgSTD.h"

/*===========================================================================*\
    CStandardQueue - Constructor
\*===========================================================================*/

CStandardQueue::CStandardQueue (tUInt MaxCountOfItems, tUInt SizeOfItem) :
            ByteQueue (MaxCountOfItems * SizeOfItem) {

    dhead ("CStandardQueue-Constructor", DCON_CStandardQueue);
    dassert (MaxCountOfItems > 0);
    dassert (SizeOfItem > 0);
    dassert ((MaxCountOfItems / 10) * (SizeOfItem / 10) <= (MAXVAL_tUInt / 100));

    sizeItem = SizeOfItem;
    maxItems = (ByteQueue.MaxSize() != 0) ? MaxCountOfItems : 0;

    dassert (ByteQueue.MaxSize() / sizeItem == maxItems);
}


/*===========================================================================*\
    CStandardQueue - Destructor
\*===========================================================================*/

CStandardQueue::~CStandardQueue () {
    dhead ("CStandardQueue-Destructor", DCON_CStandardQueue);
    maxItems = 0;
    sizeItem = 0;
}


/*===========================================================================*\
\*===========================================================================*/

tBool CStandardQueue::Put (void *dataIn) {
    dhead ("CStandardQueue::Put", DCON_CStandardQueue);
    dassert (dataIn != 0);
    dassert (sizeItem != 0);
    dwarning (maxItems != 0);

    #ifdef NDEBUG
        return (ByteQueue.Put ((tUByte *)dataIn, sizeItem) != 0) ? vTrue : vFalse;
    #else
        dinfo (ByteQueue.FreeSize() >= sizeItem, "ByteQueue to small: put=%x, free=%x",
                                                sizeItem, ByteQueue.FreeSize());
        tUInt _in = ByteQueue.Put ((tUByte *)dataIn, sizeItem);
        if (_in) {
            dassert (_in == sizeItem);
            return vTrue;
        }
        return vFalse;
    #endif
}


/*===========================================================================*\
\*===========================================================================*/

tBool CStandardQueue::PutInFront (void *dataIn) {
    dhead ("CStandardQueue::PutInFront", DCON_CStandardQueue);
    dassert (dataIn != 0);
    dassert (sizeItem != 0);
    dwarning (maxItems != 0);

    #ifdef NDEBUG
        return (ByteQueue.PutInFront ((tUByte *)dataIn, sizeItem) != 0) ? vTrue : vFalse;
    #else
        dinfo (ByteQueue.FreeSize() >= sizeItem, "ByteQueue to small: putInFront=%x, free=%x",
                                                sizeItem, ByteQueue.FreeSize());
        tUInt _in = ByteQueue.PutInFront ((tUByte *)dataIn, sizeItem);
        if (_in) {
            dassert (_in == sizeItem);
            return vTrue;
        }
        return vFalse;
    #endif
}


/*===========================================================================*\
\*===========================================================================*/

tBool CStandardQueue::Get (void *dataOut) {
    dhead ("CStandardQueue::Get", DCON_CStandardQueue);
    dassert (dataOut != 0);
    dassert (sizeItem != 0);
    dwarning (maxItems != 0);

    #ifdef NDEBUG
        return (ByteQueue.Get ((tUByte *)dataOut, sizeItem) != 0) ? vTrue : vFalse;
    #else
        dinfo (((ByteQueue.UsedSize() <= sizeItem) && ByteQueue.UsedSize()),
               "CStandardQueue: Perhaps ByteQueue lost Data: get=%x, use=%x",
               sizeItem, ByteQueue.UsedSize());
        tUInt _out = ByteQueue.Get ((tUByte *)dataOut, sizeItem);
        if (_out) {
            dassert (_out == sizeItem);
            return vTrue;
        }
        return vFalse;
    #endif
}


/*===========================================================================*\
\*===========================================================================*/

tBool CStandardQueue::Resize (tUInt newMaxCountOfItems) {
    dhead ("CStandardQueue::Resize", DCON_CStandardQueue);
    dassert (newMaxCountOfItems > 0);
    dassert ((newMaxCountOfItems / 10) * (sizeItem / 10) <= (MAXVAL_tUInt / 100));
    dassert (sizeItem != 0);
    dwarning (maxItems != 0);

    if (ByteQueue.Resize (newMaxCountOfItems * sizeItem) == vTrue) {
        maxItems = newMaxCountOfItems;
        dassert (ByteQueue.MaxSize() / sizeItem == maxItems);
        return vTrue;
    }
    return vFalse;
}


/*===========================================================================*\
\*===========================================================================*/
