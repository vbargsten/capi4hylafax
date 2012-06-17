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

#ifndef _STDARRAY_H_
#define _STDARRAY_H_

#include "aTypes.h"
#include "Protect.h"

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

typedef tResult (*tArrayCompFunc) (const void *data, const void *ItemFrag);

/*===========================================================================*\
\*===========================================================================*/

class CStandardArray {
public:
    CStandardArray (tUInt MaxCountOfItems, tUInt SizeOfItem);
    virtual ~CStandardArray (void);

    // return: 0 = error, 1 == success but no dataOut, 2 == success and dataOut valid
    tUInt Set (tUInt pos, void *dataIn, void *dataOut = 0);
    tUInt SetEmpty (void *dataIn, tUInt *pPos = 0);         // Set an item in an empty place

    virtual tBool AppendEmpty (tUInt *pPos = 0);            // Append an empty item
    virtual tBool Append (void *dataIn, tUInt *pPos = 0);
    virtual tBool Ins (tUInt pos, void *dataIn);

    tBool Get (tUInt pos, void *dataOut);
    const void *Get_Direct (tUInt pos);
    tBool GetNext (void *dataOut, tUInt *FindPos = 0, tUInt StartPos = 0);
    const void *GetNext_Direct (tUInt *FindPos = 0, tUInt StartPos = 0);

    // del removes the item from the array, all greater items will be shift one pos lower
    // for return value see above
    tUInt Del (tUInt pos, void *dataOut = 0);
    tUInt DelLast (void *dataOut = 0);
    void DelAll (void);

    // clear makes the item invalid, but the pos still exists
    // for return value see above
    tUInt Clear (tUInt pos, void *dataOut = 0);
    tUInt ClearLast (void *dataOut = 0);
    void ClearAll (void);

    tResult Find (tArrayCompFunc CompFunc, void *ItemFrag, void *dataOut = 0, tUInt *FindPos = 0, tUInt StartPos = 0);
    void *Find_Direct (tArrayCompFunc CompFunc, void *ItemFrag, tUInt StartPos = 0);  // Return: CompFunc == vYes

    tBool IsItemEmpty (tUInt pos);
    tBool IsArrayEmpty (void);
    tBool IsArrayFull (void);
    tBool ExistUsedItems (void);
    tBool ExistEmptyItems (void);
    tUInt CountUsedItems (void);
    tUInt CountEmptyItems (void);
    tUInt CountAllItems (void);
    tUInt MaxCountItems (void);
    tUInt ItemSize (void);

    tBool Resize (tUInt newMaxItemCount);

private:
    Protect_Define (hProtect);
    tUChar *Array;
    tUChar *ArrayEmpty;
    tUInt   count;
    tUInt   emptyCount;
    tUInt   maxCount;
    tUInt   sizeCopy;       // size of the item == size needed to copy
    tUInt   sizeItem;       // real used, alligned size
};

/*===========================================================================*\
\*===========================================================================*/

inline tBool CStandardArray::IsItemEmpty (tUInt pos) {
    return (tBool)(ArrayEmpty[pos] == 1);
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline tBool CStandardArray::IsArrayEmpty (void) {
    return (count) ? vFalse : vTrue;
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline tBool CStandardArray::IsArrayFull (void) {
    return (count == maxCount) ? vTrue : vFalse;
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline tBool CStandardArray::ExistUsedItems (void) {
    return (count > emptyCount) ? vTrue : vFalse;
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline tBool CStandardArray::ExistEmptyItems (void) {
    return (emptyCount) ? vTrue : vFalse;
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline tUInt CStandardArray::CountUsedItems (void) {
    return count - emptyCount;
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline tUInt CStandardArray::CountEmptyItems (void) {
    return emptyCount;
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline tUInt CStandardArray::CountAllItems (void) {
    return count;
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline tUInt CStandardArray::MaxCountItems (void) {
    return maxCount;
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline tUInt CStandardArray::ItemSize (void) {
    return sizeItem - 1;
}


/*===========================================================================*\
\*===========================================================================*/

#endif
