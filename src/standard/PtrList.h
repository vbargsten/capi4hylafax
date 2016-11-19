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

#ifndef _PTRLIST_H_
#define _PTRLIST_H_

#include "aTypes.h"

/*===========================================================================*\
\*===========================================================================*/

class CPointerListElement {
public:
    typedef tResult (*ctFindFunc) (CPointerListElement *pElement, void *RefData);

    CPointerListElement (void);
    virtual ~CPointerListElement (void);

    CPointerListElement *GetNext (void);
    CPointerListElement *GetPrev (void);
    void AddAfter (CPointerListElement *pElement);
    void AddBefore (CPointerListElement *pElement);
    void Exchange (CPointerListElement *pElement);
    void RemoveFromList (void);
    CPointerListElement *RemoveAfter (void);
    CPointerListElement *RemoveBefore (void);

    tResult FindFwd (ctFindFunc func, void *RefData, CPointerListElement **ppFoundElement = 0);  // the search begin at GetNext();
    tResult FindRev (ctFindFunc func, void *RefData, CPointerListElement **ppFoundElement = 0);  // the search begin at GetPrev();

private:
    void InternalRemove (void);

    CPointerListElement *pNext;
    CPointerListElement *pPrev;
};


/*===========================================================================*\
\*===========================================================================*/

class CPointerList {
public:
    typedef tResult (*ctFindFunc) (CPointerListElement *pElement, void *RefData);

    CPointerList (void);
    virtual ~CPointerList (void);
    tBool IsEmpty (void);
    tUInt CountElements (void);

    CPointerListElement *GetFirst (void);
    CPointerListElement *GetLast (void);
    void AddFirst (CPointerListElement *pElement);
    void AddLast (CPointerListElement *pElement);

    CPointerListElement *RemoveFirst (void);
    CPointerListElement *RemoveLast (void);
    void RemoveAndDeleteAll (void);

    tResult FindFwd (ctFindFunc func, void *RefData, CPointerListElement **ppFoundElement = 0);  // the search begin at GetFirst();
    tResult FindRev (ctFindFunc func, void *RefData, CPointerListElement **ppFoundElement = 0);  // the search begin at GetLast();

private:
    CPointerListElement Head;
    CPointerListElement Tail;
};


/*===========================================================================*\
    inline Functions for CPointerListElement
\*===========================================================================*/

#include "dbgSTD.h"

inline CPointerListElement::CPointerListElement (void)
  : pNext (0),
    pPrev (0) {
    dhead ("CPointerListElement-Constructor", DCON_CPointerList);
}

inline CPointerListElement *CPointerListElement::GetNext (void) {
    dhead ("CPointerListElement::GetNext", DCON_CPointerList);
    dassert (pNext != 0);
    // the last element is for list managing only
    CPointerListElement *fret = pNext;
    RETURN ('x', (fret->pNext) ? fret : 0);
}

inline CPointerListElement *CPointerListElement::GetPrev (void) {
    dhead ("CPointerListElement::GetPrev", DCON_CPointerList);
    dassert (pPrev != 0);
    // the first element is for list managing only
    CPointerListElement *fret = pPrev;
    RETURN ('x',(fret->pPrev) ? fret : 0);
}

inline void CPointerListElement::Exchange (CPointerListElement *pElement) {
    dhead ("CPointerListElement::Exchange", DCON_CPointerList);
    dassert (pElement != 0);
    AddBefore (pElement);
    RemoveFromList();
}

inline void CPointerListElement::RemoveFromList (void) {
    dhead ("CPointerListElement::RemoveFromList", DCON_CPointerList);
    dparams ("%x", this);
    InternalRemove();
}

inline CPointerListElement *CPointerListElement::RemoveAfter (void) {
    dhead ("CPointerListElement::RemoveAfter", DCON_CPointerList);
    CPointerListElement *fret = 0;
    if (pNext && pNext->pNext) {
        fret = pNext;
        fret->InternalRemove();
    }
    RETURN ('x', fret);
}

inline CPointerListElement *CPointerListElement::RemoveBefore (void) {
    dhead ("CPointerListElement::RemoveBefore", DCON_CPointerList);
    CPointerListElement *fret = 0;
    if (pPrev && pPrev->pPrev) {
        fret = pPrev;
        fret->InternalRemove();
    }
    RETURN ('x', fret);
}


/*===========================================================================*\
    inline Functions for CPointerList
\*===========================================================================*/

inline CPointerList::CPointerList (void) {
    dhead ("CPointerList-Constructor", DCON_CPointerList);
    // initialize the list
    Head.AddAfter (&Tail);
}

inline tBool CPointerList::IsEmpty (void) {
    dhead ("CPointerList::IsEmpty", DCON_CPointerList);
    return (Head.GetNext() == 0) ? vTrue : vFalse;
}

inline CPointerListElement *CPointerList::GetFirst (void) {
    dhead ("CPointerList::GetFirst", DCON_CPointerList);
    return Head.GetNext();
}

inline CPointerListElement *CPointerList::GetLast (void) {
    dhead ("CPointerList::GetLast", DCON_CPointerList);
    return Tail.GetPrev();
}

inline void CPointerList::AddFirst (CPointerListElement *pElement) {
    dhead ("CPointerList::AddFirst", DCON_CPointerList);
    dparams ("%x", pElement);
    Head.AddAfter (pElement);
}

inline void CPointerList::AddLast (CPointerListElement *pElement) {
    dhead ("CPointerList::AddLast", DCON_CPointerList);
    dparams ("%x", pElement);
    Tail.AddBefore (pElement);
}

inline CPointerListElement *CPointerList::RemoveFirst (void) {
    dhead ("CPointerList::RemoveFirst", DCON_CPointerList);
    return Head.RemoveAfter();
}

inline CPointerListElement *CPointerList::RemoveLast (void) {
    dhead ("CPointerList::RemoveLast", DCON_CPointerList);
    return Tail.RemoveBefore();
}

inline tResult CPointerList::FindFwd (ctFindFunc func, void *RefData, CPointerListElement **ppFoundElement) {
    dhead ("CPointerList::FindFwd", DCON_CPointerList);
    return Head.FindFwd (func, RefData, ppFoundElement);
}

inline tResult CPointerList::FindRev (ctFindFunc func, void *RefData, CPointerListElement **ppFoundElement) {
    dhead ("CPointerList::FindRev", DCON_CPointerList);
    return Tail.FindRev (func, RefData, ppFoundElement);
}

/*===========================================================================*\
\*===========================================================================*/

#endif

