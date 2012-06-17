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

#ifndef _STDLIST_H_
#define _STDLIST_H_

#include "aTypes.h"
#include "Protect.h"

/*===========================================================================*\
\*===========================================================================*/

class CListElement {
protected:
    virtual ~CListElement (void) {}

    // this virtual functions have to be implemented only if sorting is needed
    virtual tCompare Compare (void *RefCompData);   // return: RefCompData [tCompare] this
    virtual void *GetRefCompData (void);

private:
    CListElement *m_pLow;
    CListElement *m_pHigh;

    friend class CStandardList;
};


/*===========================================================================*\
\*===========================================================================*/

class CStandardList : private CListElement {
public:
    typedef tResult (*ctFindFunc) (CListElement *pElement, void *RefData);

    CStandardList (tBool AutoSort = vFalse);
    virtual ~CStandardList (void);

    tBool IsEmpty (void);
    tUInt CountElements (void);

    // Low = in the lower direction | High = in the higher direction
    // pCur == 0 => Low: pCur = highest Element, High: pCur = lowest Element
    // return vFalse => normal, everything okay
    //        vTrue  => equal Element already exist. new Element was added
    //                  Low = After, High = Before the equal Element
    tBool AddLow  (CListElement *pNew, CListElement *pCur = 0);
    tBool AddHigh (CListElement *pNew, CListElement *pCur = 0);

    tBool GetLow  (void *ppGet, CListElement *pCur = 0);
    tBool GetHigh (void *ppGet, CListElement *pCur = 0);

    // pCur is not included in the search!
    tResult FindLow  (ctFindFunc func, void *RefData = 0, void *ppFind = 0, CListElement *pCur = 0);
    tResult FindHigh (ctFindFunc func, void *RefData = 0, void *ppFind = 0, CListElement *pCur = 0);
    tResult FindLow  (void *RefCompData, void *ppFind = 0, CListElement *pCur = 0);
    tResult FindHigh (void *RefCompData, void *ppFind = 0, CListElement *pCur = 0);

    void Remove (CListElement *pCur);
    void RemoveAndDeleteAll (void);

    //void SortNow (void);    // not implemented now!

private:
    Protect_Define (m_hProtect);
    tUInt m_count;
    tBool m_AutoSort;
};


/*===========================================================================*\
    CListElement - inline functions
\*===========================================================================*/

inline tCompare CListElement::Compare (void *) {
    return vEqual;
}

inline void *CListElement::GetRefCompData (void) {
    return 0;
}


/*===========================================================================*\
    CStandardList - inline Functions
\*===========================================================================*/

inline tBool CStandardList::IsEmpty (void) {
    return (m_count == 0);
}

inline tUInt CStandardList::CountElements (void) {
    return m_count;
}

/*===========================================================================*\
\*===========================================================================*/

#endif

