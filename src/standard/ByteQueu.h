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

#ifndef _BYTEQUEU_H_
#define _BYTEQUEU_H_

#include "aTypes.h"
#include "Protect.h"


/*===========================================================================*\
    CStandardByteQueue
\*===========================================================================*/

class CStandardByteQueue {
public:
    CStandardByteQueue (tUInt maxQueueSize, tBool PartFillAllow = vFalse);
    virtual ~CStandardByteQueue (void);

    virtual tUInt Put (tUByte *dataIn, tUInt length);
    virtual tBool PutByte (tUByte byteIn);
    virtual tUInt PutInFront (tUByte *dataIn, tUInt length);
    tUInt Get (tUByte *dataOut, tUInt maxLength);
    void Clear (void);

    tUInt Show (tUByte *data, tUInt maxLength, tUInt OffsetFromEnd = 0);
    tUByte ShowByte (tUInt OffsetFromEnd = 0);
    tUInt Fill (tUByte fillByte, tUInt length = 0);  // length = 0 => fill whole queue
    tUInt Remove (tUInt maxLength);

    tBool IsEmpty (void);
    tBool IsFull (void);
    tUInt UsedSize (void);
    tUInt FreeSize (void);
    tUInt MaxSize (void);
    void  SetPartFill (tBool PartFillAllow);

    tBool Resize (tUInt newMaxQueueSize);

private:
    Protect_Define (hProtectPut);
    Protect_Define (hProtectGet);
    tUByte   *buffer;
    tUInt     maxCount;
    tUInt     head;
    tUInt     tail;
    tBool     bPartFill;
};


/*===========================================================================*\
    inline Functionen
\*===========================================================================*/

inline tBool CStandardByteQueue::IsEmpty (void) {
    return (head == tail) ? vTrue : vFalse;
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline tBool CStandardByteQueue::IsFull (void) {
    tUInt dHead = head + 1;
    tUInt dTail = tail;
    if (dHead >= maxCount) {
        dHead = 0;
    }
    return (dHead == dTail) ? vTrue : vFalse;
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline tUInt CStandardByteQueue::UsedSize (void) {
    tUInt dHead = head;
    tUInt dTail = tail;

    if (dHead < dTail) {
        dHead += maxCount;
    }
    return (dHead - dTail);
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline tUInt CStandardByteQueue::FreeSize (void) {
    if (!maxCount) {
        return 0;
    }
    tUInt dHead = head;
    tUInt dTail = tail;

    if (dHead >= dTail) {
        dTail += maxCount;
    }
    return (dTail - dHead - 1);
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline tUInt CStandardByteQueue::MaxSize (void) {
    return (maxCount) ? (maxCount - 1) : 0;
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline void CStandardByteQueue::SetPartFill (tBool PartFillAllow) {
    bPartFill = PartFillAllow;
}

/*===========================================================================*\
\*===========================================================================*/


#endif
