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

#ifndef _FTIF2HUF_H_
#define _FTIF2HUF_H_

#include <tiffio.h>
#include "InfoType.h"

/*---------------------------------------------------------------------------*\
    tSInfo - Values
\*---------------------------------------------------------------------------*/

#define iWrn_Filter_PageEnd      CalculateInfoValue (infoType_Filter, infoKind_Warning, 1)
#define iWrn_Filter_LastPageEnd  CalculateInfoValue (infoType_Filter, infoKind_Warning, 2)
#define iErr_Filter_OpenFailed   CalculateInfoValue (infoType_Filter, infoKind_Error,   1)


/*===========================================================================*\
\*===========================================================================*/

class CSFilter_fTif2Huf {
public:
    CSFilter_fTif2Huf (void);
    virtual ~CSFilter_fTif2Huf (void);

    tBool Open (tUChar *TifFileName);
    void Close (void);

    tBool CheckForCorrectFaxFile (void);

    tUInt GetToPageEnd (tUByte *buffer, tUInt maxLen, tSInfo *pInfo);
    tUInt GetStrip (tUByte *buffer, tUInt maxLen, tSInfo *pInfo);

    tUInt GetYRes (void);
    tUInt CurStripSize (void);

private:
    void SetPageInfo (void);

    TIFF   *hTiff;
    tUInt   YResolution;
    const tUChar *BitRevField;
    tUInt  *StripCountField;
    tUInt   nStrips;
    tSInt   curStrip;

    tUChar *stripBuf;
    tUInt   stripBufMaxLen;
    tUInt   stripBufLen;
    tUInt   stripBufOff;
};

/*===========================================================================*\
\*===========================================================================*/

inline tUInt CSFilter_fTif2Huf::GetYRes (void) {
    return (hTiff) ? YResolution : 0;
}

inline tUInt CSFilter_fTif2Huf::CurStripSize (void) {
    return stripBufLen - stripBufOff;
}

/*===========================================================================*\
\*===========================================================================*/

#endif

