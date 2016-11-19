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

#ifndef _INFOTYPE_H_
#define _INFOTYPE_H_

#include "aTypes.h"

/*===========================================================================*\
\*===========================================================================*/

typedef tSInt32 tSInfo;

/*===========================================================================*\
\*===========================================================================*/

inline tBool   infoSuccess (tSInfo i)       { return (i >= 0); }
inline tBool   infoWarning (tSInfo i)       { return (i > 0);  }
inline tBool   infoError   (tSInfo i)       { return (i < 0);  }
inline tUInt16 infoType    (tSInfo i)       { return (tUInt16)((((tUInt32)i) >> 16) & 0x7FFF); }
inline tUInt16 infoID      (tSInfo i)       { return (tUInt16)(i & 0xFFFF); }

/*===========================================================================*\
\*===========================================================================*/

#define infoType_General            0x0000
#define infoType_File               0x0001
#define infoType_Capi               0x0002
#define infoType_MSNList            0x0003
#define infoType_Resource           0x0004
#define infoType_Registry           0x0005
#define infoType_WinAppl            0x0006
#define infoType_Packet             0x0007
#define infoType_Filter             0x0008
#define infoType_IniFile            0x0009
#define infoType_Serial             0x000A

/*===========================================================================*\
\*===========================================================================*/

#define infoKind_Warning            0
#define infoKind_Error              1

#define CalculateInfoValue(type, kind, id)      \
    ((tSInfo)((((kind) & 0x1) << 31) + (((type) & 0x7FFF) << 16) + ((id) & 0xFFFF)))


/*===========================================================================*\
\*===========================================================================*/

#define i_Okay                      CalculateInfoValue (infoType_General, infoKind_Warning, 0)
#define i_Pending                   CalculateInfoValue (infoType_General, infoKind_Warning, 1)
#define iErr_OutOfMemory            CalculateInfoValue (infoType_General, infoKind_Error,   1)
#define iErr_QueueFull              CalculateInfoValue (infoType_General, infoKind_Error,   2)
#define iErr_WrongState             CalculateInfoValue (infoType_General, infoKind_Error,   3)

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

#define iErr_File_NotExist          CalculateInfoValue (infoType_File,    infoKind_Error,   1)
#define iErr_File_OpenFailed        CalculateInfoValue (infoType_File,    infoKind_Error,   2)
#define iErr_File_CreateFailed      CalculateInfoValue (infoType_File,    infoKind_Error,   3)

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline tSInfo MapWinErr2Info (tSInt32 val) {
    return (val) ? CalculateInfoValue (infoType_WinAppl, infoKind_Error, val) : i_Okay;
}

/*===========================================================================*\
\*===========================================================================*/

#endif

