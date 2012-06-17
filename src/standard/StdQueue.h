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

#ifndef _STDQUEUE_H_
#define _STDQUEUE_H_

#include "aTypes.h"
#include "ByteQueu.h"

/*===========================================================================*\
    CStandardQueue
\*===========================================================================*/

class CStandardQueue {
public:
    CStandardQueue (tUInt MaxCountOfItems, tUInt SizeOfItem);
    virtual ~CStandardQueue (void);

    virtual tBool Put (void *dataIn);
    virtual tBool PutInFront (void *dataIn);
    tBool Get (void *dataOut);
    void Clear (void);

    tBool IsEmpty (void);
    tUInt UsedItems (void);
    tUInt FreeItems (void);
    tUInt MaxItems (void);
    tUInt ItemSize (void);

    tBool Resize (tUInt newMaxCountOfItems);

private:
    CStandardByteQueue ByteQueue;
    tUInt              maxItems;
    tUInt              sizeItem;
};


/*===========================================================================*\
    inline functions of CStandardQueue
\*===========================================================================*/

inline void CStandardQueue::Clear (void) {
    ByteQueue.Clear();
}


/*===========================================================================*\
\*===========================================================================*/

inline tBool CStandardQueue::IsEmpty (void) {
    return ByteQueue.IsEmpty();
}


/*===========================================================================*\
\*===========================================================================*/

inline tUInt CStandardQueue::UsedItems (void) {
    return (ByteQueue.UsedSize() / sizeItem);
}


/*===========================================================================*\
\*===========================================================================*/

inline tUInt CStandardQueue::FreeItems (void) {
    return (ByteQueue.FreeSize() / sizeItem);
}


/*===========================================================================*\
\*===========================================================================*/

inline tUInt CStandardQueue::MaxItems (void) {
    return maxItems;
}


/*===========================================================================*\
\*===========================================================================*/

inline tUInt CStandardQueue::ItemSize (void) {
    return sizeItem;
}


/*===========================================================================*\
\*===========================================================================*/

#endif
