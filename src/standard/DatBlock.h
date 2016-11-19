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

#ifndef _DATBLOCK_H_
#define _DATBLOCK_H_

#include "DataPack.h"

/*===========================================================================*\
\*===========================================================================*/

class CDataPacketBlocks : public CDataPacket {
public:
    CDataPacketBlocks (tUInt BlockSize = 0);
    void SetBlockSize (tUInt BlockSize);

    void Init (CDataPacket *pPacket);
    void Init (tUByte *pPointer, tUInt Len, void *FirstHandle = 0);

    void FirstBlock (void);
    tBool NextBlock (void);
    tBool IsLastBlock (void);

private:
    tUByte *m_RealPointer;
    tUInt   m_RealMaxLen;
    tUInt   m_RealLen;
    tUInt   m_Delta;
    tUInt   m_Offset;
};

/*===========================================================================*\
    CDataPacketBlocks - inline functions
\*===========================================================================*/

inline CDataPacketBlocks::CDataPacketBlocks (tUInt BlockSize)
  : m_Delta (BlockSize),
    m_Offset (0),
    m_RealPointer (0),
    m_RealMaxLen (0),
    m_RealLen (0) {}

inline void CDataPacketBlocks::SetBlockSize (tUInt BlockSize) {
    m_Delta = BlockSize;
    FirstBlock();
}

inline void CDataPacketBlocks::Init (CDataPacket *pPacket) {
    // assert (pPacket != 0);
    m_RealPointer = pPacket->GetPointer();
    m_RealMaxLen  = pPacket->GetMaxLen();
    m_RealLen     = pPacket->GetLen();
    for (tUInt i = DATA_PACKET_HANDLE_COUNT; i > 0; i --) {
        SetHandle (pPacket->GetHandle (i), i);
    }
    FirstBlock();
}

inline void CDataPacketBlocks::Init (tUByte *pPointer, tUInt Len, void *FirstHandle) {
    m_RealPointer = pPacket->GetPointer();
    m_RealMaxLen  = pPacket->GetMaxLen();
    m_RealLen     = pPacket->GetLen();
    SetHandle (FirstHandle);
    FirstBlock();
}

inline void CDataPacketBlocks::FirstBlock (void) {
    SetPointer (m_RealPointer);
    m_Offset = 0;
    if (m_RealLen > m_Delta) {
        SetLen (m_Delta);
        SetMaxLen (m_Delta);
    } else {
        SetMaxLen (m_RealMaxLen);
        SetLen (m_RealLen);
    }
    SetPos (0);
}

inline tBool CDataPacketBlocks::NextBlock (void) {
    m_Offset += m_Delta;
    if (m_RealLen <= m_Offset) {
        FirstBlock();
        return vFalse;
    }
    SetPointer (m_RealPointer + m_Offset);
    if (m_RealLen <= m_Offset + m_Delta) {
        SetLen (m_RealLen - m_Offset);
        SetMaxLen (m_RealMaxLen - m_Offset);
    }
    SetPos (0);
    return vTrue;
}

inline tBool CDataPacketBlocks::IsLastBlock (void) {
    return (m_RealLen <= m_Offset + m_Delta) ? vTrue : vFalse;
}


/*===========================================================================*\
\*===========================================================================*/

#endif
