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

#include "StdArray.h"
#include "dbgSTD.h"
#include "aString.h"

/*===========================================================================*\
    CStandardArray - Constructor
\*===========================================================================*/

CStandardArray::CStandardArray (tUInt MaxCountOfItems, tUInt SizeOfItem)
  : Array (0),
    ArrayEmpty (0),
    count (0),
    emptyCount (0),
    maxCount (0),
    sizeCopy (SizeOfItem),
    sizeItem (SizeOfItem + (sizeof (void*) - (SizeOfItem % sizeof (void*))) % sizeof (void*)) {

    dhead   ("CStandardArray-Constructor", DCON_CStandardArray);
    // The stuff value ensure that each item will be alinged at pointer boundaries.
    // tUInt stuff = (sizeof (void*) - (SizeOfItem % sizeof (void*))) % sizeof (void*);
    dassert (MaxCountOfItems > 0);
    dassert (SizeOfItem      > 0);
    dassert ((MaxCountOfItems / 10) * ((sizeItem)/ 10) <= (MAXVAL_tUInt / 100));
    Protect_InitHandle (&hProtect);

    Resize (MaxCountOfItems);
}


/*===========================================================================*\
    CStandardArray - Destructor
\*===========================================================================*/

CStandardArray::~CStandardArray (void) {
    dhead ("CStandardArray-Destructor", DCON_CStandardArray);
    tUChar *tmpArray;
    tUChar *tmpArrayEmpty;
    Protect_BeginWrite (hProtect);
    sizeItem      = 0;
    count         = 0;
    emptyCount    = 0;
    maxCount      = 0;
    tmpArray      = Array;
    tmpArrayEmpty = ArrayEmpty;
    Array         = 0;
    Protect_EndWrite (hProtect);
    delete [] tmpArray;
    delete [] tmpArrayEmpty;
    Protect_Destroy (&hProtect);
}


/*===========================================================================*\
    CStandardArray::Set
\*===========================================================================*/

tUInt CStandardArray::Set (tUInt pos, void *dataIn, void *dataOut) {
    dhead ("CStandardArray::Set", DCON_CStandardArray);
    dparams ("%x,%p,%p", pos, dataIn, dataOut);
    dassert (dataIn != 0);
    tUInt fret = 0;
    Protect_BeginWrite (hProtect);
    if (count > pos) {
        fret++;
        if (ArrayEmpty[pos] != 1) {
            if (dataOut) {
                fret++;
                s_memcpy (dataOut, Array + pos * sizeItem, sizeCopy);
            }
        } else {
            dassert (emptyCount > 0);
            ArrayEmpty[pos] = 0;
            emptyCount--;
        }
        s_memcpy (Array + pos * sizeItem, dataIn, sizeCopy);
        Protect_EndWrite (hProtect);
    } else if (count == pos) {
        Protect_EndWrite (hProtect);
        fret = (Append (dataIn) == vFalse) ? 0 : 1;
    } else {
        Protect_EndWrite (hProtect);
    }
    RETURN ('x', fret);
}


/*===========================================================================*\
    CStandardArray::SetEmpty
\*===========================================================================*/

tUInt CStandardArray::SetEmpty (void *dataIn, tUInt *pPos) {
    dhead ("CStandardArray::SetEmpty", DCON_CStandardArray);
    dparams ("%p,%p", dataIn, pPos);
    dassert (dataIn != 0);
    Protect_BeginWrite (hProtect);
    if (emptyCount) {
        tUInt pos;
        for (tUInt i = 0; i < count; i++) {
            pos = i * sizeItem;
            if (ArrayEmpty[i] == 1) {
                emptyCount--;
                ArrayEmpty[i] = 0;
                s_memcpy (Array + pos, dataIn, sizeCopy);
                if (pPos) {
                    *pPos = i;
                }
                Protect_EndWrite (hProtect);
                return 1;
            }
        }
    }
    Protect_EndWrite (hProtect);
    return (Append (dataIn, pPos) == vFalse) ? 0 : 1;
}


/*===========================================================================*\
    CStandardArray::AppendEmpty
\*===========================================================================*/

tBool CStandardArray::AppendEmpty (tUInt *pPos) {
    dhead ("CStandardArray::AppendEmpty", DCON_CStandardArray);
    dparams ("%p", pPos);
    tBool fret = vFalse;
    Protect_BeginWrite (hProtect);
    if (count < maxCount) {
        ArrayEmpty[count] = 1;
        if (pPos) {
            *pPos = count;
        }
        emptyCount++;
        count++;
        fret = vTrue;
    }
    Protect_EndWrite (hProtect);
    RETURN ('x', fret);
}


/*===========================================================================*\
    CStandardArray::Append
\*===========================================================================*/

tBool CStandardArray::Append (void *dataIn, tUInt *pPos) {
    dhead ("CStandardArray::Append", DCON_CStandardArray);
    dparams ("%p,%p", dataIn, pPos);
    dassert (dataIn != 0);
    tBool fret = vFalse;
    Protect_BeginWrite (hProtect);
    if (count < maxCount) {
        tUInt pos = count * sizeItem;
        ArrayEmpty[count] = 0;
        s_memcpy (Array + pos, dataIn, sizeCopy);
        if (pPos) {
            *pPos = count;
        }
        count++;
        fret = vTrue;
    }
    Protect_EndWrite (hProtect);
    RETURN ('x', fret);
}


/*===========================================================================*\
    CStandardArray::Ins
\*===========================================================================*/

tBool CStandardArray::Ins (tUInt pos, void *dataIn) {
    dhead ("CStandardArray::Ins", DCON_CStandardArray);
    dparams ("%x,%p", pos, dataIn);
    tBool fret = vFalse;
    Protect_BeginWrite (hProtect);
    if (pos == count) {
        Protect_EndWrite (hProtect);
        fret = Append (dataIn);
    } else {
        if ((pos < count) && (count < maxCount)) {
            tUInt len = count - pos;
            s_strmov (ArrayEmpty + pos, sizeof (tUChar), len);
            ArrayEmpty[pos] = 0;
            pos *= sizeItem;
            s_strmov (Array + pos, sizeItem, len * sizeItem);
            s_memcpy (Array + pos, dataIn, sizeCopy);
            count++;
            fret = vTrue;
        }
        Protect_EndWrite (hProtect);
    }
    RETURN ('x', fret);
}


/*===========================================================================*\
    CStandardArray::Get
\*===========================================================================*/

tBool CStandardArray::Get (tUInt pos, void *dataOut) {
    dhead ("CStandardArray::Get", DCON_CStandardArray);
    dparams ("%x,%p", pos, dataOut);
    dassert (dataOut != 0);
    tBool fret = vFalse;
    tUInt ix   = pos;

    Protect_BeginRead (hProtect);
    if (pos < count) {
        pos *= sizeItem;
        if (ArrayEmpty[ix] != 1) {
            s_memcpy (dataOut, Array + pos, sizeCopy);
            fret = vTrue;
        }
    }
    Protect_EndRead (hProtect);
    return fret;
}


/*===========================================================================*\
    CStandardArray::Get_Direct
\*===========================================================================*/

const void *CStandardArray::Get_Direct (tUInt pos) {
    dhead ("CStandardArray::Get_Direct", DCON_CStandardArray);
    dparams ("%x", pos);
    void *fret = 0;
    tUInt ix   = pos;

    Protect_BeginRead (hProtect);
    if (pos < count) {
        pos *= sizeItem;
        if (ArrayEmpty[ix] != 1) {
            fret = Array + pos;
        }
    }
    Protect_EndRead (hProtect);
    return fret;
}


/*===========================================================================*\
    CStandardArray::GetNext
\*===========================================================================*/

tBool CStandardArray::GetNext (void *dataOut, tUInt *FindPos, tUInt StartPos) {
    dhead ("CStandardArray::GetNext", DCON_CStandardArray);
    dparams ("%p,%p,%x", dataOut, FindPos, StartPos);
    tBool fret = vFalse;
    tUInt pos  = 0;

    Protect_BeginRead (hProtect);
    for (; StartPos < count; StartPos++) {
        pos = StartPos * sizeItem;
        if (ArrayEmpty[StartPos] != 1) {
            if (dataOut) {
                s_memcpy (dataOut, Array + pos, sizeCopy);
            }
            if (FindPos) {
                *FindPos = StartPos;
            }
            fret = vTrue;
            break;
        }
    }
    Protect_EndRead (hProtect);
    RETURN ('x', fret);
}


/*===========================================================================*\
    CStandardArray::GetNext_Direct
\*===========================================================================*/

const void *CStandardArray::GetNext_Direct (tUInt *FindPos, tUInt StartPos) {
    dhead ("CStandardArray::GetNext_Direct", DCON_CStandardArray);
    dparams ("%p,%x", FindPos, StartPos);
    void *fret = 0;
    tUInt pos  = 0;

    Protect_BeginRead (hProtect);
    for (; StartPos < count; StartPos++) {
        pos = StartPos * sizeItem;
        if (ArrayEmpty[StartPos] != 1) {
            if (FindPos) {
                *FindPos = StartPos;
            }
            fret = Array + pos;
            break;
        }
    }
    Protect_EndRead (hProtect);
    return fret;
}


/*===========================================================================*\
    CStandardArray::Del
\*===========================================================================*/

tUInt CStandardArray::Del (tUInt pos, void *dataOut) {
    dhead ("CStandardArray::Del", DCON_CStandardArray);
    tUInt fret = 0;
    Protect_BeginWrite (hProtect);
    if (pos + 1 < count) {
        fret++;
        if (ArrayEmpty[pos] == 1) {
            dassert (emptyCount > 0);
            emptyCount--;
        } else if (dataOut) {
            fret++;
            s_memcpy (dataOut, Array + pos * sizeItem, sizeCopy);
        }
        s_strmov (ArrayEmpty + pos + 1, -1, count - pos - 1);
        s_strmov (Array + (pos + 1) * sizeItem, -(tSInt)sizeItem, (count - pos - 1) * sizeItem);
        count--;
        fret = vTrue;
        Protect_EndWrite (hProtect);
    } else if (pos + 1 == count) {
        Protect_EndWrite (hProtect);
        fret = DelLast (dataOut);
    } else {
        Protect_EndWrite (hProtect);
    }
    RETURN ('x', fret);
}


/*===========================================================================*\
    CStandardArray::DelLast
\*===========================================================================*/

tUInt CStandardArray::DelLast (void *dataOut) {
    dhead ("CStandardArray::DelLast", DCON_CStandardArray);
    tUInt fret = 0;
    Protect_BeginWrite (hProtect);
    if (count) {
        count--;
        fret++;
        if (ArrayEmpty[count] == 1) {
            dassert (emptyCount > 0);
            emptyCount--;
        } else if (dataOut) {
            fret++;
            s_memcpy (dataOut, Array + count * sizeItem, sizeCopy);
        }
    }
    Protect_EndWrite (hProtect);
    RETURN ('x', fret);
}


/*===========================================================================*\
    CStandardArray::DelAll
\*===========================================================================*/

void CStandardArray::DelAll (void) {
    dhead ("CStandardArray::DelAll", DCON_CStandardArray);
    Protect_BeginWrite (hProtect);
    count = 0;
    emptyCount = 0;
    Protect_EndWrite (hProtect);
}


/*===========================================================================*\
    CStandardArray::Clear
\*===========================================================================*/

tUInt CStandardArray::Clear (tUInt pos, void *dataOut) {
    dhead ("CStandardArray::Clear", DCON_CStandardArray);
    tUInt fret = 0;
    Protect_BeginWrite (hProtect);
    if (pos + 1 < count) {
        if (ArrayEmpty[pos] != 1) {
            ArrayEmpty[pos] = 1;
            emptyCount++;
            dassert (emptyCount <= count);
            fret++;
            if (dataOut) {
                fret++;
                s_memcpy (dataOut, Array + pos * sizeItem, sizeCopy);
            }
        }
        Protect_EndWrite (hProtect);
    } else if (pos + 1 == count) {
        Protect_EndWrite (hProtect);
        fret = ClearLast (dataOut);
    } else {
        Protect_EndWrite (hProtect);
    }
    RETURN ('x', fret);
}


/*===========================================================================*\
    CStandardArray::ClearLast
\*===========================================================================*/

tUInt CStandardArray::ClearLast (void *dataOut) {
    dhead ("CStandardArray::ClearLast", DCON_CStandardArray);
    tUInt fret = 0;
    Protect_BeginWrite (hProtect);
    if (count) {
        tUInt pos = count - 1;
        if (ArrayEmpty[pos] != 1) {
            ArrayEmpty[pos] = 1;
            emptyCount++;
            dassert (emptyCount <= count);
            fret++;
            if (dataOut) {
                fret++;
                s_memcpy (dataOut, Array + pos * sizeItem, sizeCopy);
            }
        }
    }
    Protect_EndWrite (hProtect);
    RETURN ('x', fret);
}


/*===========================================================================*\
    CStandardArray::ClearAll
\*===========================================================================*/

void CStandardArray::ClearAll (void) {
    dhead ("CStandardArray::ClearAll", DCON_CStandardArray);
    Protect_BeginWrite (hProtect);
    for (tUInt i = 0; i < count; i++) {
        ArrayEmpty[i] = 1;
    }
    emptyCount = count;
    Protect_EndWrite (hProtect);
}


/*===========================================================================*\
    CStandardArray::Find
\*===========================================================================*/

tResult CStandardArray::Find (tArrayCompFunc CompFunc, void *ItemFrag, void *dataOut,
                              tUInt *FindPos, tUInt StartPos) {
    dhead ("CStandardArray::Find", DCON_CStandardArray);
    dparams ("%p,%p,%p,%p,%x", CompFunc, ItemFrag, dataOut, FindPos, StartPos);
    dassert (CompFunc != 0);
    dassert (sizeItem > 0);
    dwarning (Array != 0);
    tResult fret = vNo;
    tUChar *lauf;

    Protect_BeginRead (hProtect);
    for (lauf = Array + (StartPos * sizeItem); StartPos < count; StartPos++, lauf += sizeItem) {
        if (ArrayEmpty [StartPos] != 1) {
            fret = CompFunc (lauf, ItemFrag);
            if (fret != vNo) {
                if (FindPos) {
                    *FindPos = StartPos;
                }
                if (dataOut) {
                    s_memcpy (dataOut, lauf, sizeCopy);
                }
                break;
            }
        }
    }
    Protect_EndRead (hProtect);
    RETURN ('x', fret);
}


/*===========================================================================*\
    CStandardArray::Find_Direct
\*===========================================================================*/

void *CStandardArray::Find_Direct (tArrayCompFunc CompFunc, void *ItemFrag, tUInt StartPos) {
    dhead ("CStandardArray::Find_Direct", DCON_CStandardArray);
    dparams ("%p,%p,%x", CompFunc, ItemFrag, StartPos);
    dassert (CompFunc != 0);
    dassert (sizeItem > 0);
    dwarning (Array != 0);
    tResult fret;
    Protect_BeginRead (hProtect);
    for (tUChar *lauf = Array + (StartPos * sizeItem); StartPos < count; StartPos++, lauf += sizeItem) {
        if (ArrayEmpty [StartPos] != 1) {
            fret = CompFunc (lauf, ItemFrag);
            if (fret != vNo) {
                if (fret == vYes) {
                    Protect_EndRead (hProtect);
                    RETURN ('x', lauf);
                }
                break;
            }
        }
    }
    Protect_EndRead (hProtect);
    RETURN ('x', 0);
}


/*===========================================================================*\
    CStandardArray::Resize
\*===========================================================================*/

tBool CStandardArray::Resize (tUInt newMaxItemCount) {
    dhead ("CStandardArray::Resize", DCON_CStandardArray);
    dparams ("%x,old=%x,count=%x", newMaxItemCount, maxCount, count);
    dassert (newMaxItemCount > 0);
    dassert ((newMaxItemCount / 10) * (sizeItem / 10) <= (MAXVAL_tUInt / 100));
    tBool fret = vFalse;
    if (  (Protect_IsCreated (hProtect) == vTrue)
       || (Protect_Create (&hProtect) == vTrue)) {

        tUChar *newArray      = new tUChar [newMaxItemCount * sizeItem];
        tUChar *newArrayEmpty = new tUChar [newMaxItemCount           ];
        tUChar *delArray      = newArray;
        tUChar *delArrayEmpty = newArrayEmpty;

        Protect_BeginWrite (hProtect);
        if ((count < newMaxItemCount) && newArray && newArrayEmpty) {
            if (count) {
                s_memcpy (newArray     , Array     , count * sizeItem);
                s_memcpy (newArrayEmpty, ArrayEmpty, count           );
            }
            maxCount      = newMaxItemCount;
            delArray      = Array;
            delArrayEmpty = ArrayEmpty;
            Array         = newArray;
            ArrayEmpty    = newArrayEmpty;
            fret          = vTrue;
        }
        Protect_EndWrite (hProtect);
        delete [] delArray;
        delete [] delArrayEmpty;
    }
    RETURN ('x', fret);
}


/*===========================================================================*\
\*===========================================================================*/
