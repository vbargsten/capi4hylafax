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

#include "DynArray.h"
#include "dbgSTD.h"

/*===========================================================================*\
    CDynamicArray::Append
\*===========================================================================*/

tBool CDynamicArray::Append (void *dataIn, tUInt *pos) {
    dhead ("CDynamicArray::Append", DCON_CStandardArray);
    dassert (dataIn != 0);
    if (IsArrayFull()) {
        Resize ((MaxCountItems() >= (MAXVAL_tUInt / (ItemSize() * 2))) ?
                                       (MAXVAL_tUInt / ItemSize()) :
                                       ((MaxCountItems() * 2) + 1));
    }
    return CStandardArray::Append (dataIn, pos);
}


/*===========================================================================*\
    CDynamicArray::Ins
\*===========================================================================*/

tBool CDynamicArray::Ins (tUInt pos, void *dataIn) {
    dhead ("CDynamicArray::Ins", DCON_CStandardArray);
    if (IsArrayFull()) {
        Resize ((MaxCountItems() >= (MAXVAL_tUInt / (ItemSize() * 2))) ?
                                       (MAXVAL_tUInt / ItemSize()) :
                                       ((MaxCountItems() * 2) + 1));
    }
    return CStandardArray::Ins (pos, dataIn);
}


/*===========================================================================*\
\*===========================================================================*/
