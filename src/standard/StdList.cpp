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

#include "StdList.h"
#include "dbgSTD.h"

/*===========================================================================*\
\*===========================================================================*/

CStandardList::CStandardList (tBool AutoSort)
  : m_AutoSort (AutoSort),
    m_count (0) {

    dhead ("CStandardList-Constructor", DCON_CStandardList);
    Protect_Create (&m_hProtect);
    m_pLow = m_pHigh = this;
}


/*===========================================================================*\
\*===========================================================================*/

CStandardList::~CStandardList (void) {
    dhead ("CStandardList-Destructor", DCON_CStandardList);
    RemoveAndDeleteAll();
    Protect_Destroy (&m_hProtect);
}


/*===========================================================================*\
\*===========================================================================*/

tBool CStandardList::AddLow (CListElement *pNew, CListElement *pCur) {
    dhead ("CStandardList::AddLow", DCON_CStandardList);
    dassert (pNew != 0);
    tBool fret = vFalse;
    // TODO: check if pCur is in the List
    if (m_AutoSort == vTrue) {
        void *RefData = pNew->GetRefCompData();
        // TODO: Implementieren mittels Find and Get
        Protect_BeginWrite (m_hProtect);
        if (!pCur) {
            pCur = m_pLow;
        }
        switch (pCur->Compare (RefData)) {
        case vLower:
            pCur = pCur->m_pLow;
            while (pCur->Compare (RefData) == vLower) {
                pCur = pCur->m_pLow;
            }
            pCur = pCur->m_pHigh;
            break;
        case vEqual:
            if (pCur == this) {
                break;
            }
            fret = vTrue;
            // no break!
        case vHigher:
            dprint ("-Wrong Direction- ");
            dwarning (0);
            pCur = pCur->m_pHigh;
            while ((pCur != this) && (pCur->Compare (RefData) != vLower)) {
                pCur = pCur->m_pHigh;
            }
            break;
        }
        if ((pCur->m_pLow != this) && (pCur->m_pLow->Compare (RefData) == vEqual)) {
            fret = vTrue;
        }
    } else {
        Protect_BeginWrite (m_hProtect);
        if (!pCur) {
            pCur = this;
        }
    }
    pCur->m_pLow->m_pHigh = pNew;
    pNew->m_pLow          = pCur->m_pLow;
    pNew->m_pHigh         = pCur;
    m_count++;
    Protect_EndWrite (m_hProtect);
    RETURN ('x', fret);
}


/*===========================================================================*\
\*===========================================================================*/

tBool CStandardList::AddHigh (CListElement *pNew, CListElement *pCur) {
    dhead ("CStandardList::AddHigh", DCON_CStandardList);
    dassert (pNew != 0);
    tBool fret = vFalse;
    // TODO: check if pCur is in the List
    if (m_AutoSort == vTrue) {
        void *RefData = pNew->GetRefCompData();
        // TODO: Implementieren mittels Find and Get
        Protect_BeginWrite (m_hProtect);
        if (!pCur) {
            pCur = m_pHigh;
        }
        switch (pCur->Compare (RefData)) {
        case vHigher:
            pCur = pCur->m_pHigh;
            while (pCur->Compare (RefData) == vHigher) {
                pCur = pCur->m_pHigh;
            }
            pCur = pCur->m_pLow;
            break;
        case vEqual:
            if (pCur == this) {
                break;
            }
            // no break!
        case vLower:
            dprint ("-Wrong Direction- ");
            dwarning (0);
            pCur = pCur->m_pLow;
            while ((pCur != this) && (pCur->Compare (RefData) != vHigher)) {
                pCur = pCur->m_pLow;
            }
            break;
        }
        if ((pCur->m_pHigh != this) && (pCur->m_pHigh->Compare (RefData) == vEqual)) {
            fret = vTrue;
        }
    } else {
        Protect_BeginWrite (m_hProtect);
        if (!pCur) {
            pCur = this;
        }
    }
    pCur->m_pHigh->m_pLow = pNew;
    pNew->m_pHigh         = pCur->m_pHigh;
    pNew->m_pLow          = pCur;
    m_count++;
    Protect_EndWrite (m_hProtect);
    RETURN ('x', fret);
}

/*===========================================================================*\
\*===========================================================================*/

tBool CStandardList::GetLow (void *ppGet, CListElement *pCur) {
    dhead ("CStandardList::GetLow", DCON_CStandardList);
    dassert (ppGet != 0);
    Protect_BeginRead (m_hProtect);
    pCur = (pCur) ? pCur->m_pLow : m_pLow;
    if (pCur != this) {
        *(CListElement **)ppGet = pCur;
        Protect_EndRead (m_hProtect);
        RETURN ('x', vTrue);
    }
    *(CListElement **)ppGet = 0;
    Protect_EndRead (m_hProtect);
    RETURN ('x', vFalse);
}


/*===========================================================================*\
\*===========================================================================*/

tBool CStandardList::GetHigh (void *ppGet, CListElement *pCur) {
    dhead ("CStandardList::GetHigh", DCON_CStandardList);
    dassert (ppGet != 0);
    Protect_BeginRead (m_hProtect);
    pCur = (pCur) ? pCur->m_pHigh : m_pHigh;
    if (pCur != this) {
        *(CListElement **)ppGet = pCur;
        Protect_EndRead (m_hProtect);
        RETURN ('x', vTrue);
    }
    *(CListElement **)ppGet = 0;
    Protect_EndRead (m_hProtect);
    RETURN ('x', vFalse);
}

/*===========================================================================*\
\*===========================================================================*/

tResult CStandardList::FindLow (ctFindFunc func, void *RefData, void *ppFind, CListElement *pCur) {
    dhead ("CStandardList::FindLow", DCON_CStandardList);
    tResult fret = vNo;
    while (GetLow (&pCur, pCur) == vTrue) {
        dassert (pCur != 0);
        fret = func (pCur, RefData);
        if (fret != vNo) {
            if (ppFind) {
                *(CListElement **)ppFind = pCur;
            }
            break;
        }
    }
    RETURN ('x', fret);
}

/*===========================================================================*\
\*===========================================================================*/

tResult CStandardList::FindHigh (ctFindFunc func, void *RefData, void *ppFind, CListElement *pCur) {
    dhead ("CStandardList::FindHigh", DCON_CStandardList);
    tResult fret = vNo;
    while (GetHigh (&pCur, pCur) == vTrue) {
        dassert (pCur != 0);
        fret = func (pCur, RefData);
        if (fret != vNo) {
            if (ppFind) {
                *(CListElement **)ppFind = pCur;
            }
            break;
        }
    }
    RETURN ('x', fret);
}

/*===========================================================================*\
\*===========================================================================*/

tResult CStandardList::FindLow  (void *RefCompData, void *ppFind, CListElement *pCur) {
    dhead ("CStandardList::FindLow", DCON_CStandardList);
    while (GetLow (&pCur, pCur) == vTrue) {
        dassert (pCur != 0);
        switch (pCur->Compare (RefCompData)) {
        case vEqual:
            if (ppFind) {
                *(CListElement **)ppFind = pCur;
            }
            RETURN ('x', vYes);
        case vHigher:
            if (ppFind) {
                *(CListElement **)ppFind = pCur;
            }
            RETURN ('x', vCancel);
        case vLower:
            break;
        }
    }
    RETURN ('x', vNo);
}

/*===========================================================================*\
\*===========================================================================*/

tResult CStandardList::FindHigh (void *RefCompData, void *ppFind, CListElement *pCur) {
    dhead ("CStandardList::FindLow", DCON_CStandardList);
    while (GetHigh (&pCur, pCur) == vTrue) {
        dassert (pCur != 0);
        switch (pCur->Compare (RefCompData)) {
        case vEqual:
            if (ppFind) {
                *(CListElement **)ppFind = pCur;
            }
            RETURN ('x', vYes);
        case vLower:
            if (ppFind) {
                *(CListElement **)ppFind = pCur;
            }
            RETURN ('x', vCancel);
        case vHigher:
            break;
        }
    }
    RETURN ('x', vNo);
}


/*===========================================================================*\
\*===========================================================================*/

void CStandardList::Remove (CListElement *pCur) {
    dhead ("CStandardList::Remove", DCON_CStandardList);
    dassert (pCur != 0);
    dassert (pCur->m_pHigh != 0);
    dassert (pCur->m_pLow != 0);
    dassert (m_count > 0);
    Protect_BeginWrite (m_hProtect);
    pCur->m_pLow->m_pHigh = pCur->m_pHigh;
    pCur->m_pHigh->m_pLow = pCur->m_pLow;
    m_count--;
    Protect_EndWrite (m_hProtect);
    pCur->m_pLow = 0;
    pCur->m_pHigh = 0;
}

/*===========================================================================*\
\*===========================================================================*/

void CStandardList::RemoveAndDeleteAll (void) {
    dhead ("CStandardList::RemoveAndDeleteAll", DCON_CStandardList);
    CListElement *pDel;
    Protect_BeginWrite (m_hProtect);
    while (m_pLow != this) {
        pDel             = m_pLow;
        m_pLow          = m_pLow->m_pLow;
        m_pLow->m_pHigh = this;
        delete pDel;
    }
    m_count = 0;
    dassert (m_pLow == this);
    dassert (m_pHigh == this);
    Protect_EndWrite (m_hProtect);
}

/*===========================================================================*\
\*===========================================================================*/
