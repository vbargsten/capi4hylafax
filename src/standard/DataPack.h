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

#ifndef _DATAPACK_H_
#define _DATAPACK_H_

#include "aTypes.h"
#include "aString.h"
#include "InfoType.h"

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

#ifndef DATA_PACKET_HANDLE_COUNT
#define DATA_PACKET_HANDLE_COUNT    1       // have to be 1 or greater!!!
#endif

#define iWrn_Pack_DestFull          CalculateInfoValue (infoType_Packet, infoKind_Warning, 1)
#define iWrn_Pack_BothFull          CalculateInfoValue (infoType_Packet, infoKind_Warning, 1)
#define iErr_Pack_NotAllowed        CalculateInfoValue (infoType_Packet, infoKind_Error, 1)
#define iErr_Pack_LenToSmall        CalculateInfoValue (infoType_Packet, infoKind_Error, 2)

/*===========================================================================*\
\*===========================================================================*/

class CBasicDataPacketQueue {
public:
    virtual ~CBasicDataPacketQueue (void) {}
    virtual class CDataPacket *RecvPacket (void) = 0;
    virtual void ReturnPacket (class CDataPacket *pPacket) = 0;
};


/*===========================================================================*\
\*===========================================================================*/

class CDataPacket {
public:
    CDataPacket (void);
    virtual ~CDataPacket (void) {}

    // Pointer
    tUByte *GetPointer (void);
    tUByte *GetPointer (tUInt offset);
    tUByte *GetPosPointer (void);
    tUByte *GetLenPointer (void);

    // Length
    tUInt GetLen (void);
    tUInt GetFreeLen (void);
    tUInt GetMaxLen (void);
    void  SetLen (tUInt len);
    void  SetLenToMax (void);
    void  AddLen (tUInt len);
    void  SubLen (tUInt len);
    tBool LenIsMax (void);
    tBool IsEmpty (void);

    // Position
    tUInt GetPos (void);
    tUInt DiffPosToLen (void);
    tUInt DiffPosToMaxLen (void);
    tBool SetPos (tUInt pos);
    void  SetPosToLen (void);
    tBool AddPos (tUInt pos);
    tBool PosIsLen (void);
    tBool PosIsMax (void);

    // Handle
    void *GetHandle (tUInt number = 0); // number < DATA_PACKET_HANDLE_COUNT
    void SetHandle (void *handle, tUInt number = 0); // number < DATA_PACKET_HANDLE_COUNT

    // Queue
    CDataPacket *GetNextPacket (void);
    void SetNextPacket (CDataPacket *pPacket);
    void ReturnPacket (void);
    void SetPacketQueue (CBasicDataPacketQueue *pQueue);

    // Copy
    tSInfo Copy (CDataPacket *pPacket); // return: i_Okay, iWrn_Pack_DestFull, iWrn_Pack_BothFull

    // Resize
    virtual tSInfo Resize (tUInt NewMaxLen);

protected:
    void SetPointer (tUByte *DataPointer);
    void SetMaxLen (tUInt MaxLen);

private:
    CBasicDataPacketQueue *m_pQueue;
    CDataPacket           *m_pNext;
    tUByte                *m_Pointer;
    tUInt                  m_MaxLen;
    tUInt                  m_Len;
    tUInt                  m_Pos;
    void                  *m_Handle[DATA_PACKET_HANDLE_COUNT];
};


/*===========================================================================*\
\*===========================================================================*/

class CDataPacketStore : public CDataPacket {
public:
    CDataPacketStore (void) {}
    CDataPacketStore (tUInt MaxLen);
    ~CDataPacketStore (void);

    tSInfo Resize (tUInt NewMaxLen);
};

class CDataPacketReference : public CDataPacket {
public:
    CDataPacketReference (void) {}
    CDataPacketReference (tUByte *pPointer, tUInt MaxLen, tUInt Len = 0, void *FirstHandle = 0);

    void InitToMax (tUByte *pPointer, tUInt Len);
    void InitToMax (tUByte *pPointer, tUInt Len, void *FirstHandle);
    void InitAll (tUByte *pPointer, tUInt MaxLen, tUInt Len = 0);
    void InitAll (tUByte *pPointer, tUInt MaxLen, void *FirstHandle);
    void InitAll (tUByte *pPointer, tUInt MaxLen, tUInt Len, void *FirstHandle);
};


/*===========================================================================*\
    CDataPacket - inline functions
\*===========================================================================*/

inline CDataPacket::CDataPacket (void)
  : m_pQueue  (0),
    m_pNext   (0),
    m_Pointer (0),
    m_MaxLen  (0),
    m_Len     (0),
    m_Pos     (0) {

    s_memzero (m_Handle, DATA_PACKET_HANDLE_COUNT * sizeof (void *));
}

inline tUByte *CDataPacket::GetPointer (void) {
    return m_Pointer;
}

inline tUByte *CDataPacket::GetPointer (tUInt offset) {
    return m_Pointer + offset;
}

inline tUByte *CDataPacket::GetPosPointer (void) {
    return (m_Pos < m_MaxLen) ? GetPointer (m_Pos) : 0;
}

inline tUByte *CDataPacket::GetLenPointer (void) {
    return GetPointer (m_Len);
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline tUInt CDataPacket::GetLen (void) {
    return m_Len;
}

inline tUInt CDataPacket::GetFreeLen (void) {
    return m_MaxLen - m_Len;
}

inline tUInt CDataPacket::GetMaxLen (void) {
    return m_MaxLen;
}

inline void CDataPacket::SetLen (tUInt len) {
    // assert (len <= MaxLen);
    m_Len = len;
    if (m_Pos > len) {
        SetPosToLen();
    }
}

inline void CDataPacket::SetLenToMax (void) {
    m_Len = m_MaxLen;
}

inline void CDataPacket::AddLen (tUInt len) {
    // assert (len + m_Len <= MaxLen);
    m_Len += len;
}

inline void CDataPacket::SubLen (tUInt len) {
    // assert (m_Len >= len);
    m_Len -= len;
    if (m_Pos > m_Len) {
        SetPosToLen();
    }
}

inline tBool CDataPacket::LenIsMax (void) {
    return (m_MaxLen == m_Len) ? vTrue : vFalse;
}

inline tBool CDataPacket::IsEmpty (void) {
    return (m_Len == 0);
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline tUInt CDataPacket::GetPos (void) {
    return m_Pos;
}

inline tUInt CDataPacket::DiffPosToLen (void) {
    return m_Len - m_Pos;
}

inline tUInt CDataPacket::DiffPosToMaxLen (void) {
    return m_MaxLen - m_Pos;
}

inline tBool CDataPacket::SetPos (tUInt pos) {
    if (pos < m_Len) {
        m_Pos = pos;
        return vTrue;
    }
    SetPosToLen();
    return vFalse;
}

inline void CDataPacket::SetPosToLen (void) {
    m_Pos = m_Len;
}

inline tBool CDataPacket::AddPos (tUInt pos) {
    m_Pos += pos;
    if (m_Pos <= m_Len) {
        return vTrue;
    }
    SetPosToLen();
    return vFalse;
}

inline tBool CDataPacket::PosIsLen (void) {
    return (m_Pos == m_Len);
}

inline tBool CDataPacket::PosIsMax (void) {
    return (m_Pos == m_MaxLen);
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline void *CDataPacket::GetHandle (tUInt number) {
    return m_Handle[number];
}

inline void CDataPacket::SetHandle (void *handle, tUInt number) {
    m_Handle[number] = handle;
}

inline CDataPacket *CDataPacket::GetNextPacket (void) {
    return m_pNext;
}

inline void CDataPacket::SetNextPacket (CDataPacket *pPacket) {
    m_pNext = pPacket;
}

inline void CDataPacket::ReturnPacket (void) {
    // assert (m_pQueue != 0);
    m_pQueue->ReturnPacket (this);
}

inline void CDataPacket::SetPacketQueue (CBasicDataPacketQueue *pQueue) {
    m_pQueue = pQueue;
}

inline tSInfo CDataPacket::Copy (CDataPacket *pPacket) {
    // assert (pPacket != 0);
    tUInt l1 = GetFreeLen();
    tUInt l2 = pPacket->DiffPosToLen();
    tSInfo fret;
    switch (a_compare (l1, l2)) {
    default:
        // dassert (0);
        // no break!
    case vEqual:
        fret = iWrn_Pack_BothFull;
        break;
    case vLower:
        fret = iWrn_Pack_DestFull;
        break;
    case vHigher:
        l1   = l2;
        fret = i_Okay;
        break;
    }
    s_memcpy (GetLenPointer(), pPacket->GetPosPointer(), l1);
    AddLen (l1);
    pPacket->AddPos (l1);
    return fret;
}

inline tSInfo CDataPacket::Resize (tUInt) {
    return iErr_Pack_NotAllowed;
}

inline void CDataPacket::SetPointer (tUByte *DataPointer) {
    m_Pointer = DataPointer;
}

inline void CDataPacket::SetMaxLen (tUInt MaxLen) {
    m_MaxLen = MaxLen;
    if (m_Len > MaxLen) {
        SetLen (MaxLen);
    }
}


/*===========================================================================*\
    Other CDataPacket Classes
\*===========================================================================*/

inline CDataPacketStore::CDataPacketStore (tUInt MaxLen) {
    // dassert (MaxLen > 0);
    tUByte *Buffer = new tUByte [MaxLen];
    if (Buffer) {
        SetPointer (Buffer);
        SetMaxLen (MaxLen);
    }
}

inline CDataPacketStore::~CDataPacketStore (void) {
    tUByte *Buffer = GetPointer();
    SetPointer (0);
    delete [] Buffer;
}

inline tSInfo CDataPacketStore::Resize (tUInt NewMaxLen) {
    if (NewMaxLen < GetLen()) {
        return iErr_Pack_LenToSmall;
    }
    tUByte *Buffer = 0;
    if (NewMaxLen) {
        Buffer = new tUByte [NewMaxLen];
        if (!Buffer) {
            return iErr_OutOfMemory;
        }
    }
    tUByte *oldBuf = GetPointer();
    // dassert ((GetLen() == 0) || (oldBuf != 0));
    s_memcpy (oldBuf, Buffer, GetLen());
    SetPointer (Buffer);
    SetMaxLen (NewMaxLen);
    delete [] oldBuf;
    return i_Okay;
}

inline CDataPacketReference::CDataPacketReference (tUByte *pPointer, tUInt MaxLen, tUInt Len, void *FirstHandle) {
    // dassert (DATA_PACKET_HANDLE_COUNT >= 1);
    InitAll (pPointer, MaxLen, Len, FirstHandle);
}

inline void CDataPacketReference::InitToMax (tUByte *pPointer, tUInt Len) {
    InitAll (pPointer, Len, Len);
}

inline void CDataPacketReference::InitToMax (tUByte *pPointer, tUInt Len, void *FirstHandle) {
    // dassert (DATA_PACKET_HANDLE_COUNT >= 1);
    InitAll (pPointer, Len, Len, FirstHandle);
}

inline void CDataPacketReference::InitAll (tUByte *pPointer, tUInt MaxLen, tUInt Len) {
    SetPointer (pPointer);
    SetMaxLen (MaxLen);
    SetPos (0);
    SetLen (Len);
}

inline void CDataPacketReference::InitAll (tUByte *pPointer, tUInt MaxLen, void *FirstHandle) {
    // dassert (DATA_PACKET_HANDLE_COUNT >= 1);
    SetPointer (pPointer);
    SetMaxLen (MaxLen);
    SetPos (0);
    SetLen (0);
    SetHandle (FirstHandle);
}

inline void CDataPacketReference::InitAll (tUByte *pPointer, tUInt MaxLen, tUInt Len, void *FirstHandle) {
    // dassert (DATA_PACKET_HANDLE_COUNT >= 1);
    SetPointer (pPointer);
    SetMaxLen (MaxLen);
    SetPos (0);
    SetLen (Len);
    SetHandle (FirstHandle);
}


/*===========================================================================*\
\*===========================================================================*/

#endif
