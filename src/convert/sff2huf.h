/*---------------------------------------------------------------------------*\
 * Copyright (C) 2000 AVM GmbH. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, and WITHOUT
 * ANY LIABILITY FOR ANY DAMAGES arising out of or in connection
 * with the use or performance of this software. See the
 * GNU General Public License for further details.
\*---------------------------------------------------------------------------*/

#ifndef _SFF2HUF_H_
#define _SFF2HUF_H_

#include "aTypes.h"

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

#define MAX_SFF_HUF_BUFFER_LEN          16384

/*===========================================================================*\
\*===========================================================================*/

class CSFilter_Sff2Huf {
public:
    enum ctInfo {
        i_Normal,
        i_NewPage,
        i_LastPage,
        i_BufferToSmall,
        i_StripNotComplete,
        i_Error_NoFax,
        i_Error_IllegalCoding,
        i_QueueFull
    };

    struct ctPageInfo {
        tUShort PageNumber;
        tUByte  ResolutionVertical;
        tUByte  ResolutionHorizontal;
        tUShort LineLength;
        tUShort PageLength;
    };

    /*-----------------------------------------------------------------------*/

    CSFilter_Sff2Huf (void);
    virtual ~CSFilter_Sff2Huf (void);

    tBool Open (tUInt DataBitOrder);
    void Close (void);

    ctInfo PutChar (tUChar ch);
    tUInt GetStrip (tUChar *buffer, tUInt len, ctInfo *info);
    tUInt GetToPageEnd (tUChar *buffer, tUInt maxLen, ctInfo *info);

    tUInt CurStripSize (void); // 0 = strip not completly received
    tUInt CurSize (void);
    tBool GetCurPageInfo (ctPageInfo *pPInfo);
    tBool GetLastPageInfo (ctPageInfo *pPInfo);
    tBool GetIllegalLines (tUInt *pCountIllegalLines, tUInt *pCountAllLines);
    // return vTrue = IllegalLines or Fax with no Lines detected

private:
    void rxSFFInit (tUInt DataBitOrder);
    void PutQueueData (tUChar c);
    void IgnoreQueueData (tUInt len);
    void GetQueueData (void *buf, tUInt len);
    void LookQueueData (tUInt offset, void *buf, tUInt len);
    void SetInQueueData (tUInt offset, void *buf, tUInt len);

    enum rxSFF_Status {
        rxSFF_Start,
        rxSFF_RecvDocHeader,
        rxSFF_GetDocHeader,
        rxSFF_RecvPageHeaderLen,
        rxSFF_RecvPageHeader,
        rxSFF_RecvSpecialLen,
        rxSFF_GetSpecialLen,
        rxSFF_RecvDataHeader,
        rxSFF_RecvDataHeaderEx,
        rxSFF_RecvData,
        rxSFF_End
    };
    struct RXSFF {
        rxSFF_Status Status;
        tUInt        BitOrder;
        tUShort      OffsetFirstPage;
        tUByte       OffsetPageHeader;
        tUShort      LineLen;
        tUShort      PageLen;
        tUShort      DeqLineLen;
        ctPageInfo   PageInfo;
        ctPageInfo   OldPageInfo;
        ctInfo       info;
        tUInt        CountDataHeaders;
        tUInt        CountAllLines;
        tUInt        CountIllegalLines;
        tUInt        VirtCount;
        tUInt        offset;
        tUInt        count;
        tUInt        head;
        tUInt        tail;
        tUByte       buffer[MAX_SFF_HUF_BUFFER_LEN];
    };

    RXSFF rxSff;
};

/*===========================================================================*\
\*===========================================================================*/
#endif

