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

#ifndef _DATAPOOL_H_
#define _DATAPOOL_H_

#include "DataPack.h"
#include "Protect.h"
#include "dbgSTD.h"

/*===========================================================================*\
\*===========================================================================*/

class CDataPacketQueue : public CBasicDataPacketQueue {
public:
    CDataPacketQueue (void);
    virtual ~CDataPacketQueue (void);
    tBool Init (void);

    void Put (CDataPacket *pPacket);
    void PutInFront (CDataPacket *pPacket);
    CDataPacket *Get (void);
    CDataPacket *QueueOut (void); // hängt die kompletten Daten aus der Queue aus

    CDataPacket *ShowFirst (void);
    CDataPacket *ShowLast (void);

    tBool IsEmpty (void);
    tUInt CountPackets (void);

    CDataPacket *RecvPacket (void);
    void ReturnPacket (CDataPacket *pPacket);

private:
    Protect_Define (m_hProtect);
    CDataPacket    *m_First;
    CDataPacket    *m_Last;
};


/*===========================================================================*\
\*===========================================================================*/

class CDataPacketPool : public CBasicDataPacketQueue {
public:
    CDataPacketPool (void);
    virtual ~CDataPacketPool (void);
    tBool Init (void);

    CDataPacket *Fetch (void);
    void Fill (CDataPacket *pPacket);

    void DeleteAll (void);

    tBool IsEmpty (void);
    tUInt CountPackets (void);

    CDataPacket *RecvPacket (void);
    void ReturnPacket (CDataPacket *pPacket);

private:
    Protect_Define (m_hProtect);
    CDataPacket    *m_Head;
};


/*===========================================================================*\
\*===========================================================================*/

class CDataPacketPoolStore : public CDataPacketPool {
public:
    CDataPacketPoolStore (tUInt PacketMaxLen = 0);
    void SetPacketMaxLen (tUInt PacketMaxLen);

    tBool Init (tUInt StartCount);
    CDataPacket *FetchOrNew (void);

private:
    tUInt m_PacketMaxLen;
};


/*===========================================================================*\
\*===========================================================================*/

class CDataPacketPoolReference : public CDataPacketPool {
public:
    tBool Init (tUInt StartCount);
    CDataPacket *FetchOrNew (void);
};



/*===========================================================================*\
    CDataPacketQueue - inline functions
\*===========================================================================*/

inline CDataPacketQueue::CDataPacketQueue (void)
  : m_First (0),
    m_Last  (0) {

    dhead ("CDataPacketQueue-Constructor", DCON_CDataPacket);
    Protect_InitHandle (&m_hProtect);
    Protect_Create (&m_hProtect);
}

inline tBool CDataPacketQueue::Init (void) {
    dhead ("CDataPacketQueue::Init", DCON_CDataPacket);
    return (Protect_IsCreated (m_hProtect) || Protect_Create (&m_hProtect)) ? vTrue : vFalse;
}

inline CDataPacketQueue::~CDataPacketQueue (void) {
    dhead ("CDataPacketQueue-Destructor", DCON_CDataPacket);
    dwarning (IsEmpty() == vTrue);
    CDataPacket *pDel;
    while ((pDel = Get()) != 0) {
        delete pDel;
    }
    Protect_Destroy (&m_hProtect);
}

inline void CDataPacketQueue::Put (CDataPacket *pPacket) {
    dhead ("CDataPacketQueue::Put", DCON_CDataPacket);
    dassert (pPacket != 0);
    pPacket->SetNextPacket (0);
    Protect_BeginWrite (m_hProtect);
    if (m_Last) {
        m_Last->SetNextPacket (pPacket);
    } else {
        dassert (m_First == 0);
        m_First = pPacket;
    }
    m_Last = pPacket;
    Protect_EndWrite (m_hProtect);
}


inline void CDataPacketQueue::PutInFront (CDataPacket *pPacket) {
    dhead ("CDataPacketQueue::PutInFront", DCON_CDataPacket);
    dassert (pPacket != 0);
    Protect_BeginWrite (m_hProtect);
    pPacket->SetNextPacket (m_First);
    m_First = pPacket;
    if (!m_Last) {
        dassert (pPacket->GetNextPacket() == 0);
        m_Last = m_First;
    }
    Protect_EndWrite (m_hProtect);
}


inline CDataPacket *CDataPacketQueue::Get (void) {
    dhead ("CDataPacketQueue::Get", DCON_CDataPacket);
    CDataPacket *fret = 0;
    Protect_BeginWrite (m_hProtect);
    if (m_First) {
        fret = m_First;
        m_First = m_First->GetNextPacket();
        if (m_First == 0) {
            m_Last = 0;
        }
        fret->SetNextPacket (0);
    }
    Protect_EndWrite (m_hProtect);
    return fret;
}

inline CDataPacket *CDataPacketQueue::QueueOut (void) {
    dhead ("CDataPacketQueue::QueueOut", DCON_CDataPacket);
    Protect_BeginWrite (m_hProtect);
    CDataPacket *fret = m_First;
    m_First = m_Last = 0;
    Protect_EndWrite (m_hProtect);
    return fret;
}

inline CDataPacket *CDataPacketQueue::ShowFirst (void) {
    return m_First;
}

inline CDataPacket *CDataPacketQueue::ShowLast (void) {
    return m_Last;
}

inline tBool CDataPacketQueue::IsEmpty (void) {
    dhead ("CDataPacketQueue::IsEmpty", DCON_CDataPacket);
    return (m_First) ? vFalse : vTrue;
}

inline tUInt CDataPacketQueue::CountPackets (void) {
    tUInt count = 0;
    Protect_BeginRead (m_hProtect);
    for (CDataPacket *pLauf = m_First; pLauf != 0; pLauf = pLauf->GetNextPacket(), count++)
        ;
    Protect_EndRead (m_hProtect);
    return count;
}

inline CDataPacket *CDataPacketQueue::RecvPacket (void) {
    return Get();
}

inline void CDataPacketQueue::ReturnPacket (CDataPacket *pPacket) {
    Put (pPacket);
}


/*===========================================================================*\
    CDataPacketPool - inline functions
\*===========================================================================*/

inline CDataPacketPool::CDataPacketPool (void) : m_Head (0) {
    dhead ("CDataPacketPool-Constructor", DCON_CDataPacket);
    Protect_InitHandle (&m_hProtect);
    Protect_Create (&m_hProtect);
}

inline CDataPacketPool::~CDataPacketPool (void) {
    dhead ("CDataPacketPool-Destructor", DCON_CDataPacket);
    DeleteAll();
    Protect_Destroy (&m_hProtect);
}

inline tBool CDataPacketPool::Init (void) {
    dhead ("CDataPacketPool::Init", DCON_CDataPacket);
    return (Protect_IsCreated (m_hProtect) || Protect_Create (&m_hProtect)) ? vTrue : vFalse;
}

inline CDataPacket *CDataPacketPool::Fetch (void) {
    dhead ("CDataPacketPool::Fetch", DCON_CDataPacket);
    CDataPacket *fret = 0;
    Protect_BeginWrite (m_hProtect);
    if (m_Head) {
        fret   = m_Head;
        m_Head = m_Head->GetNextPacket();
        fret->SetNextPacket (0);
    }
    Protect_EndWrite (m_hProtect);
    return fret;
}

inline void CDataPacketPool::Fill (CDataPacket *pPacket) {
    dhead ("CDataPacketPool::Fill", DCON_CDataPacket);
    // assert (pPacket != 0);
    Protect_BeginWrite (m_hProtect);
    pPacket->SetNextPacket (m_Head);
    m_Head = pPacket;
    Protect_EndWrite (m_hProtect);
}

inline void CDataPacketPool::DeleteAll (void) {
    dhead ("CDataPacketPool::DeleteAll", DCON_CDataPacket);
    CDataPacket *pDel;
    while ((pDel = Fetch()) != 0) {
        delete pDel;
    }
}

inline tBool CDataPacketPool::IsEmpty (void) {
    dhead ("CDataPacketPool::IsEmpty", DCON_CDataPacket);
    return (m_Head) ? vFalse : vTrue;
}

inline tUInt CDataPacketPool::CountPackets (void) {
    tUInt count = 0;
    Protect_BeginRead (m_hProtect);
    for (CDataPacket *pLauf = m_Head; pLauf != 0; pLauf = pLauf->GetNextPacket(), count++)
        ;
    Protect_EndRead (m_hProtect);
    return count;
}

inline CDataPacket *CDataPacketPool::RecvPacket (void) {
    return Fetch();
}

inline void CDataPacketPool::ReturnPacket (CDataPacket *pPacket) {
    Fill (pPacket);
}


/*===========================================================================*\
    CDataPacketPoolStore - inline functions
\*===========================================================================*/

inline CDataPacketPoolStore::CDataPacketPoolStore (tUInt PacketMaxLen)
  : m_PacketMaxLen (PacketMaxLen) {
}

inline void CDataPacketPoolStore::SetPacketMaxLen (tUInt PacketMaxLen) {
    m_PacketMaxLen = PacketMaxLen;
}

inline tBool CDataPacketPoolStore::Init (tUInt StartCount) {
    if (CDataPacketPool::Init() == vFalse) {
        return vFalse;
    }
    for (; StartCount > 0; StartCount--) {
        CDataPacket *newone = new CDataPacketStore (m_PacketMaxLen);
        if (!newone || (newone->GetMaxLen() < m_PacketMaxLen)) {
            delete newone;
            DeleteAll();
            return vFalse;
        }
        newone->SetPacketQueue (this);
        Fill (newone);
    }
    return vTrue;
}

inline CDataPacket *CDataPacketPoolStore::FetchOrNew (void) {
    CDataPacket *fret = Fetch();
    if (!fret) {
        fret = new CDataPacketStore (m_PacketMaxLen);
        if (!fret || (fret->GetMaxLen() < m_PacketMaxLen)) {
            delete fret;
            fret = 0;
        } else {
            fret->SetPacketQueue (this);
        }
    }
    return fret;
}


/*===========================================================================*\
    CDataPacketPoolRef - inline functions
\*===========================================================================*/

inline tBool CDataPacketPoolReference::Init (tUInt StartCount) {
    for (; StartCount > 0; StartCount--) {
        CDataPacket *newone = new CDataPacketReference;
        if (!newone) {
            DeleteAll();
            return vFalse;
        }
        newone->SetPacketQueue (this);
        Fill (newone);
    }
    return vTrue;
}

inline CDataPacket *CDataPacketPoolReference::FetchOrNew (void) {
    CDataPacket *fret = Fetch();
    if (!fret) {
        fret = new CDataPacketReference;
        if (fret) {
            fret->SetPacketQueue (this);
        }
    }
    return fret;
}


/*===========================================================================*\
\*===========================================================================*/

#endif

