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

#include "PtrList.h"
#include "dbgSTD.h"

/*===========================================================================*\
\*===========================================================================*/

CPointerListElement::~CPointerListElement (void) {
    dhead ("CPointerListElement-Destructor", DCON_CPointerList);
    if (pNext && pPrev) {
        // It crashes if RemoveFromList is called for "Head" or "Tail"
        RemoveFromList();
    }
}

void CPointerListElement::AddAfter (CPointerListElement *pElement) {
    dhead ("CPointerListElement::AddAfter", DCON_CPointerList);
    dparams ("%x", pElement);
    dassert (pElement != 0);
    dassert (pElement->pNext == 0);
    dassert (pElement->pPrev == 0);
    pElement->pPrev = this;
    if (pNext) {
        pNext->pPrev    = pElement;
        pElement->pNext = pNext;
    }
    pNext = pElement;
}

void CPointerListElement::AddBefore (CPointerListElement *pElement) {
    dhead ("CPointerListElement::AddBefore", DCON_CPointerList);
    dparams ("%x", pElement);
    dassert (pElement != 0);
    dassert (pElement->pNext == 0);
    dassert (pElement->pPrev == 0);
    pElement->pNext = this;
    dassert (pPrev != 0);
    pPrev->pNext    = pElement;
    pElement->pPrev = pPrev;
    pPrev           = pElement;
}

tResult CPointerListElement::FindFwd (CPointerListElement::ctFindFunc func, void *RefData, CPointerListElement **ppFoundElement) {
    dhead ("CPointerListElement::FindFwd", DCON_CPointerList);
    tResult fret;
    for (CPointerListElement *pLauf = GetNext(); (pLauf != 0); pLauf = pLauf->GetNext()) {
        fret = func (pLauf, RefData);
        switch (fret) {
        case vYes:
        case vCancel:
        case vError:
            if (ppFoundElement) {
                *ppFoundElement = pLauf;
            }
            return fret;
        default:
            dassert (0);
            // no break!
        case vNo:
            // Nothing to do
            break;
        }
    }
    if (ppFoundElement) {
        *ppFoundElement = 0;
    }
    return vNo;
}

tResult CPointerListElement::FindRev (CPointerListElement::ctFindFunc func, void *RefData, CPointerListElement **ppFoundElement) {
    dhead ("CPointerListElement::FindRev", DCON_CPointerList);
    tResult fret;
    for (CPointerListElement *pLauf = GetPrev(); (pLauf != 0); pLauf = pLauf->GetNext()) {
        fret = func (pLauf, RefData);
        switch (fret) {
        case vYes:
        case vCancel:
        case vError:
            if (ppFoundElement) {
                *ppFoundElement = pLauf;
            }
            return fret;
        default:
            dassert (0);
            // no break!
        case vNo:
            // Nothing to do
            break;
        }
    }
    if (ppFoundElement) {
        *ppFoundElement = 0;
    }
    return vNo;
}

void CPointerListElement::InternalRemove (void) {
    dhead ("CPointerListElement::InternalRemove", DCON_CPointerList);
    if (pPrev != 0) {
        dassert (pNext != 0);
        dassert (pPrev->pNext == this);
        dassert (pNext->pPrev == this);
        pPrev->pNext = pNext;
        pNext->pPrev = pPrev;
        pNext = 0;
        pPrev = 0;
    } else {
        dassert (pNext == 0);
    }
}


/*===========================================================================*\
\*===========================================================================*/

CPointerList::~CPointerList (void) {
    dhead ("CPointerList-Destructor", DCON_CPointerList);
    RemoveAndDeleteAll();
    dassert (CountElements() == 0);
}

tUInt CPointerList::CountElements (void) {
    dhead ("CPointerList::CountElements", DCON_CPointerList);
    tUInt count = 0;
    for (CPointerListElement *lauf = GetFirst(); lauf != 0; lauf = lauf->GetNext()) {
        count++;
    }
    RETURN ('x', count);
}

void CPointerList::RemoveAndDeleteAll (void) {
    dhead ("CPointerList::RemoveAndDeleteAll", DCON_CPointerList);
    CPointerListElement *pDelItem;
    while ((pDelItem = GetLast()) != 0) {
        delete pDelItem;
    }
}

/*===========================================================================*\
\*===========================================================================*/
