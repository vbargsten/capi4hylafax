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

#include "DynStore.h"
#include "dbgSTD.h"

/*===========================================================================*\
\*===========================================================================*/

CDynamicStore::CDynamicStore (void)
  : pStore (0),
    dataSize (0) {
    dhead ("CDynamicStore-Constructor", DCON_CDynamicStore);
    m_Protect.Init();

    #ifndef NDEBUG
        count = 0;
    #endif
}


/*===========================================================================*\
\*===========================================================================*/

CDynamicStore::~CDynamicStore (void) {
    dhead ("CDynamicStore-Destructor", DCON_CDynamicStore);
    #ifndef NDEBUG
        dassert (count == 0);
    #endif
    tUByte *delOne;

    m_Protect.BeginWrite();
    while (pStore) {
        delOne = (tUByte *)pStore;
        pStore = pStore->next;

        m_Protect.EndWrite();
        delete [] delOne;
        m_Protect.BeginWrite();
    }
    m_Protect.EndWrite();
}


/*===========================================================================*\
\*===========================================================================*/

tBool CDynamicStore::Init (tUInt SizeOfData, tUInt StartCount) {
    dhead ("CDynamicStore-Constructor", DCON_CDynamicStore);
    dassert (SizeOfData > 0);
    if (m_Protect.Init() == vFalse) {
        RETURN ('x', vFalse);
    }
    Element *newOne;
    dataSize  = SizeOfData + sizeof (Element *);
    pStore    = 0;
    #ifndef NDEBUG
        count = 0;
    #endif
    for (; StartCount > 0; StartCount--) {
        newOne = (Element *)new tUByte [dataSize];
        if (!newOne) {
            break;
        }
        m_Protect.BeginWrite();
        newOne->next = pStore;
        pStore       = newOne;
        m_Protect.EndWrite();
    }
    RETURN ('x', vTrue);
}


/*===========================================================================*\
\*===========================================================================*/

DynStorePointer CDynamicStore::Alloc (void) {
    dhead ("CDynamicStore::Alloc", DCON_CDynamicStore);
    dassert (dataSize > 0);
    m_Protect.BeginWrite();
    Element *allocOne = pStore;
    if (pStore) {
        pStore = pStore->next;
        #ifndef NDEBUG
            count++;
        #endif
        m_Protect.EndWrite();
    } else {
        m_Protect.EndWrite();
        allocOne = (Element *)new tUByte [dataSize];
        if (!allocOne) {
            return 0;
        }
        #ifndef NDEBUG
            else {
                m_Protect.BeginWrite();
                count++;
                m_Protect.EndWrite();
            }
        #endif
    }
    return allocOne->data;
}


/*===========================================================================*\
\*===========================================================================*/

void CDynamicStore::Free (DynStorePointer pData) {
    dhead ("CDynamicStore::Free", DCON_CDynamicStore);
    dassert (dataSize > 0);
    if (pData) {
        dassert (pData != 0);
        pData -= sizeof (Element *);

        m_Protect.BeginWrite();
        ((Element *)pData)->next = pStore;
        pStore = (Element *)pData;
        #ifndef NDEBUG
            dassert (count > 0);
            count--;
        #endif
        m_Protect.EndWrite();
    }
}


/*===========================================================================*\
\*===========================================================================*/

CDynStoreQueue::CDynStoreQueue (void)
  : first (0),
    last (0) {

    dhead ("CDynStoreQueue-Constructor", DCON_CDynamicStore);
}


/*===========================================================================*\
\*===========================================================================*/

CDynStoreQueue::~CDynStoreQueue (void) {
    dhead ("CDynStoreQueue-Destructor", DCON_CDynamicStore);
    FreeQueue();
}


/*===========================================================================*\
\*===========================================================================*/

void CDynStoreQueue::InQueue (DynStorePointer pData) {
    dhead ("CDynStoreQueue::InQueue", DCON_CDynamicStore);
    dassert (pData != 0);
    pData -= sizeof (Element *);
    ((Element *)pData)->next = 0;
    m_Protect.BeginWrite();
    if (last) {
        last->next = ((Element *)pData);
    }
    last = (Element *)pData;
    if (first == 0) {
        first = last;
    }
    m_Protect.EndWrite();
}


/*===========================================================================*\
\*===========================================================================*/

DynStorePointer CDynStoreQueue::OutQueue (void) {
    dhead ("CDynStoreQueue::OutQueue", DCON_CDynamicStore);
    DynStorePointer fret = 0;
    m_Protect.BeginWrite();
    if (first) {
        fret  = first->data;
        first = first->next;
        if (first == 0) {
            last = 0;
        }
    }
    m_Protect.EndWrite();
    return fret;
}


/*===========================================================================*\
\*===========================================================================*/

void CDynStoreQueue::FreeQueue (void) {
    dhead ("CDynStoreQueue::FreeQueue", DCON_CDynamicStore);
    DynStorePointer delOne = OutQueue();
    while (delOne) {
        Free (delOne);
        delOne = OutQueue();
    }
    dassert (first == 0);
    dassert (last == 0);
}


/*===========================================================================*\
\*===========================================================================*/
