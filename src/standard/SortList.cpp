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

#include "SortList.h"
#include "dbgSTD.h"

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

static tResult Find2HigherCmpFunc (CSortPListElement *pElement, void *RefData) {
    dhead ("Find2HigherCmpFunc", DCON_CPointerList);
    dassert (pElement != 0);
    switch (pElement->Compare (RefData)) {
    case vHigher:
        return vNo;
    case vEqual:
        return vYes;
    default:
        dassert (0);
        // no break!
    case vLower:
        break;
    }
    return vCancel;
}

static tResult Find2LowerCmpFunc (CSortPListElement *pElement, void *RefData) {
    dhead ("Find2LowerCmpFunc", DCON_CPointerList);
    dassert (pElement != 0);
    switch (pElement->Compare (RefData)) {
    case vLower:
        return vNo;
    case vEqual:
        return vYes;
    default:
        dassert (0);
        // no break!
    case vHigher:
        break;
    }
    return vCancel;
}

/*===========================================================================*\
\*===========================================================================*/

CSortPListElement *CSortPListElement::Add (CSortPListElement *pElement) {
    dhead ("CSortPListElement::Add", DCON_CPointerList);
    dassert (pElement != 0);
    void (CPointerListElement:: *addOne) (CPointerListElement *) = 0;
    CSortPListElement *(CSortPListElement:: *nextOne) (void) = 0;
    tCompare           comp;
    tCompare           direction = vEqual;
    CSortPListElement *pLauf;
    CSortPListElement *pLast     = this;
    void              *CompData  = pElement->GetRefCompData();
    switch (Compare (CompData)) {
    case vLower:
        direction = vLower;
        nextOne   = &CSortPListElement::GetLower;
        addOne    = &CSortPListElement::AddBefore;
        break;
    case vHigher:
        direction = vHigher;
        nextOne   = &CSortPListElement::GetHigher;
        addOne    = &CSortPListElement::AddAfter;
        break;
    case vEqual:
        // it isn't allowed to add two identical Elements to the List
        // add the new one and return the identical item, so the user can handle this
        Exchange (pElement);
        return this;
    }
    dassert (nextOne != 0);
    dassert (addOne != 0);
    dassert (direction != vEqual);
    pLauf = (this->*nextOne)();

    // find the place where to add the new element
    while (pLauf != 0) {
        comp = pLauf->Compare (CompData);
        if (comp != direction) {
            if (comp == vEqual) {
                // it isn't allowed to add two identical Elements to the List
                // add the new one and return the identical item, so the user can handle this
                pLauf->Exchange (pElement);
                return pLauf;
            }
            break;
        }
        pLast = pLauf;
        pLauf = (pLauf->*nextOne)();
    }
    (pLast->*addOne) (pElement);
    return 0;
}

/*===========================================================================*\
\*===========================================================================*/

CSortPListElement *CSortPListElement::Find (void *FindData) {
    dhead ("CSortPListElement::Find", DCON_CPointerList);
    CPointerListElement *pFound = this;
    switch (Compare (FindData)) {
    case vLower:
        if (FindRev ((ctFindFunc)Find2LowerCmpFunc, FindData, &pFound) != vYes) {
            pFound = 0;
        } else {
            dassert (pFound != 0);
        }
        break;
    case vHigher:
        if (FindFwd ((ctFindFunc)Find2HigherCmpFunc, FindData, &pFound) != vYes) {
            pFound = 0;
        } else {
            dassert (pFound != 0);
        }
        break;
    case vEqual:
        // nothing to do
        break;
    }
    RETURN ('x', (CSortPListElement *)pFound);
}

/*===========================================================================*\
\*===========================================================================*/

CSortPListElement *CSortPointerList::FindHigher (void *CompData) {
    dhead ("CSortPointerList::FindHigher", DCON_CPointerList);
    CPointerListElement *pFound;
    if (FindFwd ((ctFindFunc)Find2HigherCmpFunc, CompData, &pFound) != vYes) {
        pFound = 0;
    }
    return (CSortPListElement *)pFound;
}

/*===========================================================================*\
\*===========================================================================*/

CSortPListElement *CSortPointerList::FindLower (void *CompData) {
    dhead ("CSortPointerList::FindLower", DCON_CPointerList);
    CPointerListElement *pFound;
    if (FindRev ((ctFindFunc)Find2LowerCmpFunc, CompData, &pFound) != vYes) {
        pFound = 0;
    }
    return (CSortPListElement *)pFound;
}

/*===========================================================================*\
\*===========================================================================*/
