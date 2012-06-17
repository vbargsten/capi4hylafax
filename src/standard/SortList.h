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

#ifndef _SORTLIST_H_
#define _SORTLIST_H_

#include "PtrList.h"

/*===========================================================================*\
\*===========================================================================*/

class CSortPListElement : protected CPointerListElement {
public:
    CSortPListElement *GetLower (void);
    CSortPListElement *GetHigher (void);
    void RemoveFromList (void);

    CSortPListElement *Add (CSortPListElement *pElement);
    // return: 0    : okay
    //         != 0 : an equal element was found, pElement is added to the list
    //                the equal element is removed from list and is the return value
    CSortPListElement *Find (void *FindData);

    virtual tCompare Compare (void *RefCompData) = 0;  // RefCompData is [tCompare] with class own's RefCompData
    virtual void *GetRefCompData (void) = 0;

    friend class CSortPointerList;
};

/*===========================================================================*\
\*===========================================================================*/

class CSortPointerList : protected CPointerList {
public:
    tBool IsEmpty (void);
    tUInt CountElements (void);
    CSortPListElement *GetHighest (void);
    CSortPListElement *GetLowest (void);

    CSortPListElement *AddFromHigh (CSortPListElement *pElement);
    CSortPListElement *AddFromLow (CSortPListElement *pElement);
    // Search for the correct place begin at lowest or highest element
    // return: 0    : okay
    //         != 0 : an equal element was found, pElement is added to the list
    //                equal element ist removed from list and is the return value

    CSortPListElement *FindHigher (ctFindFunc func, void *RefData);
    CSortPListElement *FindLower (ctFindFunc func, void *RefData);
    CSortPListElement *FindHigher (void *FindData);         // started by GetLowest()
    CSortPListElement *FindLower (void *FindData);          // started by GetHighest()

    void RemoveAndDeleteAll (void);
};

/*===========================================================================*\
    inline functions from CSortPListElement
\*===========================================================================*/

inline CSortPListElement *CSortPListElement::GetLower (void) {
    return (CSortPListElement *)CPointerListElement::GetPrev();
}

inline CSortPListElement *CSortPListElement::GetHigher (void) {
    return (CSortPListElement *)CPointerListElement::GetNext();
}

inline void CSortPListElement::RemoveFromList (void) {
    CPointerListElement::RemoveFromList();
}

/*===========================================================================*\
    inline functions from CSortPointerList
\*===========================================================================*/

inline tBool CSortPointerList::IsEmpty (void) {
    return CPointerList::IsEmpty();
}

inline tUInt CSortPointerList::CountElements (void) {
    return CPointerList::CountElements();
}

inline CSortPListElement *CSortPointerList::GetHighest (void) {
    return (CSortPListElement *)CPointerList::GetLast();
}

inline CSortPListElement *CSortPointerList::GetLowest (void) {
    return (CSortPListElement *)CPointerList::GetFirst();
}

inline CSortPListElement *CSortPointerList::AddFromHigh (CSortPListElement *pElement) {
    CSortPListElement *pH = GetHighest();
    if (pH) {
        return pH->Add (pElement);
    }
    AddFirst (pElement);
    return 0;
}

inline CSortPListElement *CSortPointerList::AddFromLow (CSortPListElement *pElement) {
    CSortPListElement *pH = GetLowest();
    if (pH) {
        return pH->Add (pElement);
    }
    AddLast (pElement);
    return 0;
}

inline CSortPListElement *CSortPointerList::FindHigher (ctFindFunc func, void *RefData) {
    return (CSortPListElement *)CPointerList::FindFwd (func, RefData);
}

inline CSortPListElement *CSortPointerList::FindLower (ctFindFunc func, void *RefData) {
    return (CSortPListElement *)CPointerList::FindRev (func, RefData);
}

inline void CSortPointerList::RemoveAndDeleteAll (void) {
    CPointerList::RemoveAndDeleteAll();
}

/*===========================================================================*\
\*===========================================================================*/

#endif
