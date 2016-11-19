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

#ifndef _DATALIST_H_
#define _DATALIST_H_

#include "DataPack.h"

/*===========================================================================*\
\*===========================================================================*/

class CDataPacketList {
public:
    CDataPacketList (void);
    virtual ~CDataPacketList (void);
    tBool IsEmpty (void);

    CDataPacket *GetFirst (void);
    CDataPacket *GetLast (void);
    CDataPacket *GetNum (tUInt number);

    void AddFirst (CDataPacket *pPacket);
    void AddLast (CDataPacket *pPacket);

    CDataPacket *RemoveFirst (void);
    void RemoveAndDeleteAll (void);

private:
    CDataPacket *m_pFirst;
    CDataPacket *m_pLast;
};


/*===========================================================================*\
\*===========================================================================*/

inline CDataPacketList::CDataPacketList (void)
  : m_pFirst (0),
    m_pLast (0) {
}

inline CDataPacketList::~CDataPacketList (void) {
    RemoveAndDeleteAll();
}

inline tBool CDataPacketList::IsEmpty (void) {
    return (m_pFirst == 0);
}

inline CDataPacket *CDataPacketList::GetFirst (void) {
    return m_pFirst;
}

inline CDataPacket *CDataPacketList::GetLast (void) {
    return m_pLast;
}

inline CDataPacket *CDataPacketList::GetNum (tUInt number) {
    CDataPacket *pPacket = m_pFirst;
    for (; (pPacket != 0) && (number > 0); number--, pPacket = pPacket->GetNextPacket()) {
        // nothing to do in this loop
    }
    return pPacket;
}

inline void CDataPacketList::AddFirst (CDataPacket *pPacket) {
    // dassert (pPacket != 0);
    // dassert (pPacket->GetNextPacket() == 0);
    if (!m_pFirst) {
        // dassert (m_pLast == 0);
        m_pLast = pPacket;
    } else {
        pPacket->SetNextPacket (m_pFirst);
    }
    m_pFirst = pPacket;
}

inline void CDataPacketList::AddLast (CDataPacket *pPacket) {
    // dassert (pPacket != 0);
    // dassert (pPacket->GetNextPacket() == 0);
    if (!m_pLast) {
        // dassert (m_pFirst == 0);
        m_pFirst = pPacket;
    } else {
        // dassert (m_pLast->GetNextPacket() == 0);
        m_pLast->SetNextPacket (pPacket);
    }
    m_pLast = pPacket;
}

inline CDataPacket *CDataPacketList::RemoveFirst (void) {
    CDataPacket *pRemPack = m_pFirst;
    if (m_pFirst) {
        m_pFirst = m_pFirst->GetNextPacket();
        if (!m_pFirst) {
            // dassert (m_pLast == pRemPacket);
            m_pLast = 0;
        } else {
            pRemPacket->SetNextPacket (0);
        }
    }
    return pRemPack;
}

inline void CDataPacketList::RemoveAndDeleteAll (void) {
    CDataPacket *pDelPacket
    while (m_pFirst) {
        pDelPacket = m_pFirst;
        m_pFirst   = m_pFirst->GetNextPacket();
        delete pDelPacket;
    }
    m_pLast = 0;
}


/*===========================================================================*\
\*===========================================================================*/

#endif

