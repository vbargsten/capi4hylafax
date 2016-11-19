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

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

union tHUnion {
    void   *p;
    tChar  *c;
    tHandle h;
    tUByte  b;
    tUInt32 u;
    tSInt32 s;
    tUInt16 u16;
    tSInt16 s16;
#ifdef C_PLATFORM_64
    tUInt64 u64;
    tSInt64 s64;

    tHUnion (tUInt64 v) { u64 = v; }
    tHUnion (tSInt64 v) { s64 = v; }
#endif
    tHUnion (void)      { p   = (void *)0; }
    tHUnion (void   *v) { p   = v; }
    tHUnion (tChar  *v) { c   = v; }
    tHUnion (tHandle v) { h   = v; }
    tHUnion (tUByte  v) { b   = v; }
    tHUnion (tUInt32 v) { u   = v; }
    tHUnion (tSInt32 v) { s   = v; }
    tHUnion (tUInt16 v) { u16 = v; }
    tHUnion (tSInt16 v) { s16 = v; }
};


/*===========================================================================*\
\*===========================================================================*/

class CBasicDataPacketQueue {
public:
    virtual ~CBasicDataPacketQueue (void) {}
    virtual class CDataPacket *RecvPacket (void) = 0;
    virtual void ReturnPacket (class CDataPacket *pPacket, tSInfo info) = 0;
};


/*===========================================================================*\
\*===========================================================================*/

class CDataPacket {
public:
    CDataPacket (void);
    virtual ~CDataPacket (void) {}

    // Pointer
    tUByte *GetPointer (void);
    tUByte *GetPointer (tSize offset);
    tUByte *GetPosPointer (void);
    tUByte *GetLenPointer (void);

    // Length
    tSize GetLen (void);
    tSize GetFreeLen (void);
    tSize GetMaxLen (void);
    void  SetLen (tSize len);
    void  SetLenToMax (void);
    void  AddLen (tOffset len);
    void  SubLen (tOffset len);
    void  IncLen (void);
    void  DecLen (void);
    tBool LenIsMax (void);
    tBool IsEmpty (void);

    // Position
    tSize GetPos (void);
    tSize DiffPosToLen (void);
    tSize DiffPosToMaxLen (void);
    tBool SetPos (tSize pos);
    void  SetPosToLen (void);
    tBool AddPos (tSize pos);
    tBool PosIsLen (void);
    tBool PosIsMax (void);

    // Handle
    tHUnion GetHandle (tUInt number = 0); // number < DATA_PACKET_HANDLE_COUNT
    void SetHandle (tHUnion handle, tUInt number = 0); // number < DATA_PACKET_HANDLE_COUNT

    // Queue
    CDataPacket *GetNextPacket (void);
    void SetNextPacket (CDataPacket *pPacket);
    void ReturnPacket (tSInfo info = i_Okay);
    void SetPacketQueue (CBasicDataPacketQueue *pQueue);

    // Copy
    tSInfo Copy (CDataPacket *pPacket); // return: i_Okay, iWrn_Pack_DestFull, iWrn_Pack_BothFull

    // Resize
    virtual tSInfo Resize (tSize NewMaxLen);

protected:
    void SetPointer (tUByte *DataPointer);
    void SetMaxLen (tSize MaxLen);

private:
    CBasicDataPacketQueue *m_pQueue;
    CDataPacket           *m_pNext;
    tUByte                *m_Pointer;
    tSize                  m_MaxLen;
    tSize                  m_Len;
    tSize                  m_Pos;
    tHUnion                m_Handle[DATA_PACKET_HANDLE_COUNT];
};


/*===========================================================================*\
\*===========================================================================*/

class CDataPacketStore : public CDataPacket {
public:
    CDataPacketStore (void) {}
    CDataPacketStore (tSize MaxLen);
    ~CDataPacketStore (void);

    tSInfo Resize (tSize NewMaxLen);
};

class CDataPacketReference : public CDataPacket {
public:
    CDataPacketReference (void) {}
    CDataPacketReference (tUByte *pPointer, tSize MaxLen, tSize Len = 0, tHUnion FirstHandle = 0);

    void InitToMax (tUByte *pPointer, tSize Len);
    void InitToMax (tUByte *pPointer, tSize Len, tHUnion FirstHandle);
    void InitAll (tUByte *pPointer, tSize MaxLen, tSize Len = 0);
    void InitAll (tUByte *pPointer, tSize MaxLen, tHUnion FirstHandle);
    void InitAll (tUByte *pPointer, tSize MaxLen, tSize Len, tHUnion FirstHandle);
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

    s_memzero (m_Handle, DATA_PACKET_HANDLE_COUNT * sizeof (tHUnion));
}

inline tUByte *CDataPacket::GetPointer (void) {
    return m_Pointer;
}

inline tUByte *CDataPacket::GetPointer (tSize offset) {
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

inline tSize CDataPacket::GetLen (void) {
    return m_Len;
}

inline tSize CDataPacket::GetFreeLen (void) {
    return m_MaxLen - m_Len;
}

inline tSize CDataPacket::GetMaxLen (void) {
    return m_MaxLen;
}

inline void CDataPacket::SetLen (tSize len) {
    // assert (len <= MaxLen);
    m_Len = len;
    if (m_Pos > len) {
        SetPosToLen();
    }
}

inline void CDataPacket::SetLenToMax (void) {
    m_Len = m_MaxLen;
}

inline void CDataPacket::AddLen (tOffset len) {
    // assert (len + m_Len <= MaxLen);
    m_Len += len;
}

inline void CDataPacket::SubLen (tOffset len) {
    // assert (m_Len >= len);
    m_Len -= len;
    if (m_Pos > m_Len) {
        SetPosToLen();
    }
}

inline void CDataPacket::IncLen (void) {
    // assert (m_Len + 1<= MaxLen);
    m_Len++;
}

inline void CDataPacket::DecLen (void) {
    // assert (m_Len > 0);
    m_Len--;
}

inline tBool CDataPacket::LenIsMax (void) {
    return (m_MaxLen == m_Len) ? vTrue : vFalse;
}

inline tBool CDataPacket::IsEmpty (void) {
    return (m_Len == 0);
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline tSize CDataPacket::GetPos (void) {
    return m_Pos;
}

inline tSize CDataPacket::DiffPosToLen (void) {
    return m_Len - m_Pos;
}

inline tSize CDataPacket::DiffPosToMaxLen (void) {
    return m_MaxLen - m_Pos;
}

inline tBool CDataPacket::SetPos (tSize pos) {
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

inline tBool CDataPacket::AddPos (tSize pos) {
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

inline tHUnion CDataPacket::GetHandle (tUInt number) {
    return m_Handle[number];
}

inline void CDataPacket::SetHandle (tHUnion handle, tUInt number) {
    m_Handle[number] = handle;
}

inline CDataPacket *CDataPacket::GetNextPacket (void) {
    return m_pNext;
}

inline void CDataPacket::SetNextPacket (CDataPacket *pPacket) {
    m_pNext = pPacket;
}

inline void CDataPacket::ReturnPacket (tSInfo info) {
    // assert (m_pQueue != 0);
    m_pQueue->ReturnPacket (this, info);
}

inline void CDataPacket::SetPacketQueue (CBasicDataPacketQueue *pQueue) {
    m_pQueue = pQueue;
}

inline tSInfo CDataPacket::Copy (CDataPacket *pPacket) {
    // assert (pPacket != 0);
    tSize l1 = GetFreeLen();
    tSize l2 = pPacket->DiffPosToLen();
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

inline tSInfo CDataPacket::Resize (tSize) {
    return iErr_Pack_NotAllowed;
}

inline void CDataPacket::SetPointer (tUByte *DataPointer) {
    m_Pointer = DataPointer;
}

inline void CDataPacket::SetMaxLen (tSize MaxLen) {
    m_MaxLen = MaxLen;
    if (m_Len > MaxLen) {
        SetLen (MaxLen);
    }
}


/*===========================================================================*\
    Other CDataPacket Classes
\*===========================================================================*/

inline CDataPacketStore::CDataPacketStore (tSize MaxLen) {
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

inline tSInfo CDataPacketStore::Resize (tSize NewMaxLen) {
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

inline CDataPacketReference::CDataPacketReference (tUByte *pPointer, tSize MaxLen, tSize Len, tHUnion FirstHandle) {
    // dassert (DATA_PACKET_HANDLE_COUNT >= 1);
    InitAll (pPointer, MaxLen, Len, FirstHandle);
}

inline void CDataPacketReference::InitToMax (tUByte *pPointer, tSize Len) {
    InitAll (pPointer, Len, Len);
}

inline void CDataPacketReference::InitToMax (tUByte *pPointer, tSize Len, tHUnion FirstHandle) {
    // dassert (DATA_PACKET_HANDLE_COUNT >= 1);
    InitAll (pPointer, Len, Len, FirstHandle);
}

inline void CDataPacketReference::InitAll (tUByte *pPointer, tSize MaxLen, tSize Len) {
    SetPointer (pPointer);
    SetMaxLen (MaxLen);
    SetPos (0);
    SetLen (Len);
}

inline void CDataPacketReference::InitAll (tUByte *pPointer, tSize MaxLen, tHUnion FirstHandle) {
    // dassert (DATA_PACKET_HANDLE_COUNT >= 1);
    SetPointer (pPointer);
    SetMaxLen (MaxLen);
    SetPos (0);
    SetLen (0);
    SetHandle (FirstHandle);
}

inline void CDataPacketReference::InitAll (tUByte *pPointer, tSize MaxLen, tSize Len, tHUnion FirstHandle) {
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
