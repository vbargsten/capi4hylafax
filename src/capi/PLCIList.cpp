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

#include "PLCIList.h"
#include "SortList.h"
#include "Protect.h"
#include "dbgCAPI.h"
#include <string.h>

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

class CPLCIListerElement : public CSortPListElement {
public:
    tUInt  Counter;
    tUByte Controller;
    tUByte PLCIField[32];

    CPLCIListerElement (void) {
        Counter    = 0;
        Controller = 0;
        memset (PLCIField, '\0', 32 * sizeof (tUByte));
    }

    tCompare Compare (void *RefCompData) {
        if (((tUByte)(tULong)RefCompData) < Controller) {
            return vLower;
        } else if (((tUByte)(tULong)RefCompData) == Controller) {
            return vEqual;
        }
        return vHigher;
    }
    void *GetRefCompData (void) {
        return (void *)(tULong)Controller;
    }
};


/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

Protect_DefineInit (static hProtect);

static CSortPointerList *pPLCIList = 0;


/*===========================================================================*\
\*===========================================================================*/

tBool InitPLCIList (void) {
    dhead ("InitPLCIList", DCON_PLCIList);
    dassert (pPLCIList == vIllegalHandle);
    if (  (Protect_IsCreated (hProtect) == vFalse)
       && (Protect_Create (&hProtect) == vFalse)) {
        RETURN ('x', vFalse);
    }
    pPLCIList = new CSortPointerList;
    if (pPLCIList) {
        RETURN ('x', vTrue);
    }
    pPLCIList = 0;
    Protect_Destroy (&hProtect);
    RETURN ('x', vFalse);
}


/*===========================================================================*\
\*===========================================================================*/

void DeinitPLCIList (void) {
    dhead ("DeinitPLCIList", DCON_PLCIList);
    dassert (pPLCIList != 0);
    dassert (pPLCIList->IsEmpty() == vTrue);
    delete pPLCIList;
    pPLCIList = 0;
    Protect_Destroy (&hProtect);
}


/*===========================================================================*\
\*===========================================================================*/

tBool TestAndSetPLCIEntry (tUInt PLCI) {
    dhead ("TestAndSetPLCI", DCON_PLCIList);
    dparams ("%x", PLCI);
    dassert (PLCI > 0);
    dassert (PLCI <= 0xFFFF);
    dwarning (pPLCIList != 0);
    tBool fret = vFalse;
    if (pPLCIList != 0) {
        tUByte cntrl    = (tUByte)(PLCI & 0x7F);
        tUByte fieldPos = (tUByte)((PLCI >> 11) & 0x1F);
        tUByte fieldBit = (tUByte)(1 << ((PLCI >> 8) & 0x7));
        dassert (cntrl != 0);
        dassert (fieldBit != 0);
        Protect_BeginWrite (hProtect);
        CPLCIListerElement *pElement = (CPLCIListerElement *)pPLCIList->FindHigher ((void *)(tULong)cntrl);
        if (pElement == 0) {
            pElement = new CPLCIListerElement;
            if (pElement) {
                pElement->Counter             = 1;
                pElement->Controller          = cntrl;
                pElement->PLCIField[fieldPos] = fieldBit;
                pPLCIList->AddFromLow (pElement);
                fret = vTrue;
            }
        } else {
            if (!(pElement->PLCIField[fieldPos] & fieldBit)) {
                pElement->Counter++;
                pElement->PLCIField[fieldPos] |= fieldBit;
                fret = vTrue;
            }
        }
        Protect_EndWrite (hProtect);
    }
    RETURN ('x', fret);
}


/*===========================================================================*\
\*===========================================================================*/

void UnsetPLCIEntry (tUInt PLCI) {
    dhead ("UnsetPLCIEntry", DCON_PLCIList);
    dparams ("%x", PLCI);
    dassert (PLCI > 0);
    dassert (PLCI <= 0xFFFF);
    tUByte fieldPos = (tUByte)((PLCI >> 11) & 0x1F);
    tUByte fieldBit = (tUByte)(1 << ((PLCI >> 8) & 0x7));
    dassert ((PLCI & 0x7F) != 0);
    dassert (fieldBit != 0);
    dassert (pPLCIList != 0);
    if (pPLCIList != 0) {
        Protect_BeginWrite (hProtect);
        CPLCIListerElement *pElement = (CPLCIListerElement *)pPLCIList->FindHigher ((void *)(tULong)(PLCI & 0x7F));
        if (pElement && (pElement->PLCIField[fieldPos] & fieldBit)) {
            dassert (pElement->Counter > 0);
            pElement->Counter--;
            if (pElement->Counter == 0) {
                delete pElement;
            } else {
                pElement->PLCIField[fieldPos] &= ~fieldBit;
            }
        }
        Protect_EndWrite (hProtect);
    }
}


/*===========================================================================*\
\*===========================================================================*/
