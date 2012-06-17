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

#include "DataHand.h"
#include "dbgCAPI.h"

/*===========================================================================*\
    CDataHandleAdmin - Constructor
\*===========================================================================*/

CDataHandleAdmin::CDataHandleAdmin (void)
  : m_Array (0),
    m_maxEntries (0),
    m_curEntries (0),
    m_nextOne (0) {

    dhead ("CDataHandleAdmin-Constructor", DCON_CTransferChannel);
    m_Protect.Init();
}


/*===========================================================================*\
    CDataHandleAdmin - Destructor
\*===========================================================================*/

CDataHandleAdmin::~CDataHandleAdmin (void) {
    dhead ("CDataHandleAdmin-Destructor", DCON_CTransferChannel);
    if (m_Protect.IsInitialized() == vTrue) {
        m_Protect.BeginWrite();
        HandleArray *tmpArray = m_Array;
        m_nextOne    = 0;
        m_curEntries = 0;
        m_maxEntries = 0;
        m_Array      = 0;
        m_Protect.EndWrite();
        delete [] tmpArray;
    } else {
        dassert (m_Array == 0);
    }
}


/*===========================================================================*\
    CDataHandleAdmin::New
\*===========================================================================*/

tBool CDataHandleAdmin::InitEntries (tUShort MaxEntries) {
    dhead ("CDataHandleAdmin::New", DCON_CTransferChannel);
    dassert (MaxEntries > 0);
    dassert (m_Array == 0);
    tBool fret = vFalse;
    if (m_Protect.Init()) {
        m_Array = new HandleArray [MaxEntries];
        if (m_Array) {
            m_Array[MaxEntries - 1].pointer = 0;
            m_Array[MaxEntries - 1].inuse   = vFalse;
            m_Array[MaxEntries - 1].next    = 0;
            for (tUShort i = (tUShort)(MaxEntries - 1); i > 0; i--) {
                m_Array[i - 1].pointer = 0;
                m_Array[i - 1].inuse   = vFalse;
                m_Array[i - 1].next    = (tUShort) (i + 1);
            }
            m_nextOne    = 1;
            m_maxEntries = MaxEntries;
            fret         = vTrue;
        }
    }
    RETURN ('x', fret);
}


/*===========================================================================*\
    CDataHandleAdmin::New
\*===========================================================================*/

tBool CDataHandleAdmin::New (void *NewDataHandle, tUShort *RetHandle) {
    dhead ("CDataHandleAdmin::New", DCON_CTransferChannel);
    dassert (m_curEntries < m_maxEntries);
    dassert (RetHandle != 0);
    dassert (m_nextOne <= m_maxEntries);
    tBool fret = vFalse;

    m_Protect.BeginWrite();
    if (m_nextOne) {
        tUShort get          = (tUShort)(m_nextOne - 1);
        m_nextOne            = m_Array[get].next;
        m_Array[get].next    = 0;
        m_Array[get].inuse   = vTrue;
        m_Array[get].pointer = NewDataHandle;
        *RetHandle           = get;
        fret                 = vTrue;
        m_curEntries++;
    }
    m_Protect.EndWrite();
    RETURN ('x', fret);
}


/*===========================================================================*\
    CDataHandleAdmin::Free
\*===========================================================================*/

tBool CDataHandleAdmin::Free (tUShort InHandle, void **RetDataHandle) {
    dhead ("CDataHandleAdmin::Free", DCON_CTransferChannel);
    dassert (RetDataHandle != 0);
    dassert (InHandle < m_maxEntries);
    dassert (m_curEntries > 0);
    dassert (m_nextOne <= m_maxEntries);
    dassert (m_Array[InHandle].inuse == vTrue);
    dassert (m_Array[InHandle].next == 0);

    tBool fret = vTrue;
    m_Protect.BeginWrite();
    if ((InHandle < m_maxEntries) && (m_Array[InHandle].inuse == vTrue)) {
        *RetDataHandle            = m_Array[InHandle].pointer;
        m_Array[InHandle].pointer = 0;
        m_Array[InHandle].inuse   = vFalse;
        m_Array[InHandle].next    = m_nextOne;
        m_nextOne                 = (tUShort)(InHandle + 1);
        m_curEntries--;
    } else {
        fret = vFalse;
    }
    m_Protect.EndWrite();
    RETURN ('x', fret);
}


/*===========================================================================*\
    CDataHandleAdmin::DelNextEntry
\*===========================================================================*/

tBool CDataHandleAdmin::DelNextEntry (void **HandleToDel) {
    dhead ("CDataHandleAdmin::DelNextEntry", DCON_CTransferChannel);
    dassert (m_nextOne <= m_maxEntries);
    tBool fret = vFalse;

    m_Protect.BeginWrite();
    if (m_curEntries) {
        tUShort lauf;
        for (tUShort i = 1; i <= m_maxEntries; i++) {
            lauf = m_nextOne;
            while (lauf != 0) {
                if (lauf == i) {
                    break;
                } else {
                    lauf = m_Array[lauf - 1].next;
                    dassert (lauf <= m_maxEntries);
                    dinfo (lauf <= m_maxEntries, "lauf=%x ", lauf);
                }
            }
            if (lauf == 0) {
                if (HandleToDel) {
                    dwarning (m_Array[i - 1].pointer != 0);
                    *HandleToDel = m_Array[i - 1].pointer;
                }
                m_Array[i - 1].pointer = 0;
                m_Array[i - 1].inuse   = vFalse;
                m_Array[i - 1].next    = m_nextOne;
                m_nextOne              = i;
                fret                   = vTrue;
                m_curEntries--;
                break;
            }
        }
    }
    m_Protect.EndWrite();
    RETURN ('x', fret);
}


/*===========================================================================*\
\*===========================================================================*/
