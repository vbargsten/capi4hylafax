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

#include "ByteQueu.h"
#include "aString.h"
#include "dbgSTD.h"

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

#define ByteQueue_PutLock           0
#define ByteQueue_GetLock           1


/*===========================================================================*\
    CStandardByteQueue - Konstruktor
\*===========================================================================*/

CStandardByteQueue::CStandardByteQueue (tUInt maxQueueSize, tBool PartFillAllow)
  : buffer (0),
    maxCount (0),
    head (0),
    tail (0) {

    dhead ("CStandardByteQueue-Constructor", DCON_CStandardByteQueue);
    dassert (maxQueueSize > 0);
    Protect_InitHandle (&hProtectGet);
    Protect_InitHandle (&hProtectPut);
    SetPartFill (PartFillAllow);
    Resize (maxQueueSize);
}


/*===========================================================================*\
    CStandardByteQueue - Destruktor
\*===========================================================================*/

CStandardByteQueue::~CStandardByteQueue (void) {
    dhead ("CStandardByteQueue-Destructor", DCON_CStandardByteQueue);
    if (Protect_IsCreated (hProtectPut) && Protect_IsCreated (hProtectGet)) {
        tUByte *tmpBuf = buffer;
        Protect_BeginWrite (hProtectPut);
        Protect_BeginWrite (hProtectGet);
        maxCount = 0;
        head = 0;
        tail = 0;
        buffer = 0;
        Protect_EndWrite (hProtectGet);
        Protect_EndWrite (hProtectPut);
        delete [] tmpBuf;
    } else {
        dassert (maxCount == 0);
        dassert (buffer == 0);
    }
    Protect_Destroy (&hProtectPut);
    Protect_Destroy (&hProtectGet);
}


/*===========================================================================*\
    CStandardByteQueue::Put
\*===========================================================================*/

tUInt CStandardByteQueue::Put (tUByte *dataIn, tUInt length) {
    dhead ("CStandardByteQueue::Put", DCON_CStandardByteQueue);
    dparams ("data=%p, len=%x", dataIn, length);
    dassert (dataIn != 0);
    dwarning (length != 0);
    dwarning (maxCount > 1);
    dwarning (buffer != 0);
    if (!maxCount) {
        RETURN ('x', 0);
    }
    tUInt tail_free = tail;

    dprint ("h=%x,t=%x", head, tail);
    Protect_BeginWrite (hProtectPut);
    if (tail_free <= head) {
        tail_free += maxCount;
    }
    tail_free -= head + 1;
    if (length > tail_free) {
        dprint ("Queue full: in=%x, free=%x!", length, tail_free);
        length = (bPartFill == vTrue) ? tail_free : 0;
    }
    if (length) {
        if (length + head >= maxCount) {
            tUInt dif = maxCount - head;
            s_memcpy (buffer + head, dataIn, dif);
            head = length - dif;
            s_memcpy (buffer, dataIn + dif, head);
        } else {
            s_memcpy (buffer + head, dataIn, length);
            head += length;
        }
    }
    Protect_EndWrite (hProtectPut);
    dprint ("h=%x,t=%x", head, tail);
    RETURN ('x', length);
}


/*===========================================================================*\
    CStandardByteQueue::PutByte
\*===========================================================================*/

tBool CStandardByteQueue::PutByte (tUByte byteIn) {
    dhead ("CStandardByteQueue::PutByte", DCON_CStandardByteQueue);
    dparams ("data=%x", byteIn);
    tUInt tmpTail = tail;
    if (tmpTail == 0) {
        tmpTail = maxCount;
    }
    dprint ("h=%x,t=%x", head, tail);
    dassert (head < maxCount);
    Protect_BeginWrite (hProtectPut);
    if (tmpTail >= head + 1) {
        Protect_EndWrite (hProtectPut);
        RETURN ('x', vFalse);
    }
    buffer[head++] = byteIn;
    if (head >= maxCount) {
        dassert (head == maxCount);
        head = 0;
    }
    Protect_EndWrite (hProtectPut);
    dprint ("h=%x,t=%x", head, tail);
    RETURN ('x', vTrue);
}


/*===========================================================================*\
    CStandardByteQueue::PutInFront
\*===========================================================================*/

tUInt CStandardByteQueue::PutInFront (tUByte *dataIn, tUInt length) {
    dhead ("CStandardByteQueue::PutInFront", DCON_CStandardByteQueue);
    dparams ("data=%p, len=%x", dataIn, length);
    dassert (dataIn != 0);
    dwarning (length != 0);
    dwarning (maxCount > 1);
    dwarning (buffer != 0);
    if (!maxCount) {
        RETURN ('x', 0);
    }
    tUInt free;
    tUInt offset = 0;

    dprint ("h=%x,t=%x", head, tail);
    Protect_BeginWrite (hProtectPut);
    Protect_BeginWrite (hProtectGet);
    free = ((tail <= head) ? maxCount : 0) + tail - head - 1;
    if (length > free) {
        dprint ("Queue full: in=%x, free=%x!", length, free);
        offset = length - free;
        length = (bPartFill == vTrue) ? free : 0;
    }

    if (tail < length) {
        tUInt dif = length - tail;
        s_memcpy (buffer, dataIn + offset + dif, tail);
        tail = maxCount - dif;
        s_memcpy (buffer + tail, dataIn + offset, dif);
    } else {
        s_memcpy (buffer + tail - length, dataIn + offset, length);
        tail -= length;
    }
    Protect_EndWrite (hProtectGet);
    Protect_EndWrite (hProtectPut);
    dprint ("h=%x,t=%x", head, tail);
    RETURN ('x', length);
}


/*===========================================================================*\
    CStandardByteQueue::Get
\*===========================================================================*/

tUInt CStandardByteQueue::Get (tUByte *dataOut, tUInt maxLength) {
    dhead ("CStandardByteQueue::Get", DCON_CStandardByteQueue);
    dparams ("data=%p, len=%x", dataOut, maxLength);
    dassert (dataOut != 0);
    dwarning (maxLength > 0);
    dwarning (maxCount > 1);
    dwarning (buffer != 0);
    tUInt head_count = head;

    dprint ("h=%x,t=%x", head, tail);
    Protect_BeginWrite (hProtectGet);
    if (head_count < tail) {
        head_count += maxCount;
    }
    head_count -= tail;
    if (maxLength > head_count) {
        dprint ("Not enough data: out=%x, use=%x!", maxLength, head_count);
        maxLength = (bPartFill == vTrue) ? head_count : 0;
    }
    if (maxLength + tail >= maxCount) {
        tUInt dif = maxCount - tail;
        s_memcpy (dataOut, buffer + tail, dif);
        tail = maxLength - dif;
        s_memcpy (dataOut + dif, buffer, tail);
    } else {
        s_memcpy (dataOut, buffer + tail, maxLength);
        tail += maxLength;
    }
    Protect_EndWrite (hProtectGet);
    dprint ("h=%x,t=%x", head, tail);
    RETURN ('x', maxLength);
}


/*===========================================================================*\
    CStandardByteQueue::Clear
\*===========================================================================*/

void CStandardByteQueue::Clear (void) {
    dhead ("CStandardByteQueue::Clear", DCON_CStandardByteQueue);
    dwarning (buffer != 0);

    Protect_BeginWrite (hProtectPut);
    Protect_BeginWrite (hProtectGet);
    head = 0;
    tail = 0;
    Protect_EndWrite (hProtectGet);
    Protect_EndWrite (hProtectPut);
}


/*===========================================================================*\
    CStandardByteQueue::Show
\*===========================================================================*/

tUInt CStandardByteQueue::Show (tUByte *data, tUInt maxLength, tUInt OffsetFromEnd) {
    dhead ("CStandardByteQueue::Show", DCON_CStandardByteQueue);
    dassert (data != 0);
    dassert (maxLength > 0);
    dwarning (maxCount > 1);
    dwarning (buffer != 0);
    tUInt head_count = head;

    Protect_BeginRead (hProtectGet);
    if (head_count < tail) {
        head_count += maxCount;
    }
    head_count -= tail;
    if (maxLength + OffsetFromEnd > head_count) {
        dprint ("Not enough data: out=%x, use=%x!", maxLength, head_count);
        maxLength = (bPartFill == vTrue) ? head_count : 0;
    }
    OffsetFromEnd += tail;
    if (OffsetFromEnd >= maxCount) {
        OffsetFromEnd -= maxCount;
    }
    if (maxLength + OffsetFromEnd >= maxCount) {
        tUInt dif = maxCount - OffsetFromEnd;
        s_memcpy (data, buffer + OffsetFromEnd, dif);
        s_memcpy (data + dif, buffer, maxLength - dif);
    } else {
        s_memcpy (data, buffer + OffsetFromEnd, maxLength);
    }
    Protect_EndRead (hProtectGet);
    RETURN ('x', maxLength);
}


/*===========================================================================*\
    CStandardByteQueue::ShowByte
\*===========================================================================*/

tUByte CStandardByteQueue::ShowByte (tUInt OffsetFromEnd) {
    dhead ("CStandardByteQueue::ShowByte", DCON_CStandardByteQueue);
    dassert (OffsetFromEnd < maxCount);
    dwarning (maxCount > 1);
    tUByte fret;
    Protect_BeginRead (hProtectGet);
    OffsetFromEnd += tail;
    if (OffsetFromEnd >= maxCount) {
        OffsetFromEnd -= maxCount;
    }
    dassert (OffsetFromEnd < head);
    fret = buffer[OffsetFromEnd];
    Protect_EndRead (hProtectGet);
    RETURN ('x', fret);
}


/*===========================================================================*\
    CStandardByteQueue::Fill
\*===========================================================================*/

tUInt CStandardByteQueue::Fill (tUByte fillByte, tUInt length) {
    dhead ("CStandardByteQueue::Fill", DCON_CStandardByteQueue);
    dparams ("fill=%c, len=%x", fillByte, length);
    dwarning (maxCount > 1);
    dwarning (buffer != 0);
    if (!maxCount) {
        RETURN ('x', 0);
    }
    tUInt tail_free = tail;

    dprint ("h=%x,t=%x", head, tail);
    Protect_BeginWrite (hProtectPut);
    if (tail_free <= head) {
        tail_free += maxCount;
    }
    tail_free -= head + 1;
    if (!length) {
        length = tail_free;
    } else if (length > tail_free) {
        dprint ("Queue full: in=%x, free=%x!", length, tail_free);
        length = (bPartFill == vTrue) ? tail_free : 0;
    }
    if (length) {
        if (length + head >= maxCount) {
            tUInt dif = maxCount - head;
            s_strset (buffer + head, fillByte, dif);
            head = length - dif;
            s_strset (buffer, fillByte, head);
        } else {
            s_strset (buffer + head, fillByte, length);
            head += length;
        }
    }
    Protect_EndWrite (hProtectPut);
    dprint ("h=%x,t=%x", head, tail);
    RETURN ('x', length);
}


/*===========================================================================*\
    CStandardByteQueue::Remove
\*===========================================================================*/

tUInt CStandardByteQueue::Remove (tUInt maxLength) {
    dhead ("CStandardByteQueue::Remove", DCON_CStandardByteQueue);
    dassert (maxLength > 0);
    dwarning (maxCount > 1);
    dwarning (buffer != 0);
    tUInt head_count = head;

    dprint ("h=%x,t=%x", head, tail);
    Protect_BeginWrite (hProtectGet);
    if (head_count < tail) {
        head_count += maxCount;
    }
    head_count -= tail;
    if (maxLength > head_count) {
        dprint ("Not enough data: out=%x, use=%x!", maxLength, head_count);
        maxLength = (bPartFill == vTrue) ? head_count : 0;
    }
    tail += maxLength;
    if (tail >= maxCount) {
        tail -= maxCount;
        dassert (tail < maxCount);
    }
    Protect_EndWrite (hProtectGet);
    dprint ("h=%x,t=%x", head, tail);
    RETURN ('x', maxLength);
}


/*===========================================================================*\
    CStandardByteQueue::Resize
\*===========================================================================*/

tBool CStandardByteQueue::Resize (tUInt newMaxQueueSize) {
    dhead ("CStandardByteQueue::Resize", DCON_CStandardByteQueue);
    dparams ("newSize=%x", newMaxQueueSize);
    dassert (newMaxQueueSize > 0);
    tBool fret = vFalse;
    if (  (  (Protect_IsCreated (hProtectPut) == vTrue)
          || (Protect_Create (&hProtectPut) == vTrue))
       && (  (Protect_IsCreated (hProtectGet) == vTrue)
          || (Protect_Create (&hProtectGet) == vTrue))) {

        tUByte *newBuf = new tUByte [newMaxQueueSize + 1];
        tUByte *tmpBuf = newBuf;

        if (newBuf) {
            Protect_BeginWrite (hProtectPut);
            Protect_BeginWrite (hProtectGet);
            tUInt count = ((head < tail) ?  maxCount : 0) + head - tail;
            if (count <= newMaxQueueSize) {
                if (head < tail) {
                    tUInt dif = maxCount - tail;
                    s_memcpy (newBuf, buffer + tail, dif);
                    s_memcpy (newBuf + dif, buffer, head);
                } else {
                    s_memcpy (newBuf, buffer + tail, head - tail);
                }
                tmpBuf = buffer;
                buffer = newBuf;
                head = count;
                tail = 0;
                maxCount = newMaxQueueSize + 1;
                fret = vTrue;
            }
            Protect_EndWrite (hProtectGet);
            Protect_EndWrite (hProtectPut);
            delete [] tmpBuf;
        }
    }
    RETURN ('x', fret);
}


/*===========================================================================*\
\*===========================================================================*/
